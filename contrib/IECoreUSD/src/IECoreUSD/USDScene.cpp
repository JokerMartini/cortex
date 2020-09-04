//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "USDScene.h"

#include "DataAlgo.h"
#include "PrimitiveAlgo.h"

#include "IECoreScene/Camera.h"
#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/SpherePrimitive.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/gf/matrix3d.h"
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/usd/usd/collectionAPI.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/basisCurves.h"
#include "pxr/usd/usdGeom/bboxCache.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/usd/usdGeom/pointInstancer.h"
#include "pxr/usd/usdGeom/points.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/camera.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/format.hpp"
#include "boost/functional/hash.hpp"

#include <iostream>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

/// \todo Use the standard PXR_VERSION instead. We can't do that until
/// everyone is using USD 19.11 though, because prior to that PXR_VERSION
/// was malformed (octal, and not comparable in any way).
#define USD_VERSION ( PXR_MAJOR_VERSION * 10000 + PXR_MINOR_VERSION * 100 + PXR_PATCH_VERSION )

namespace
{

void convertPath( SceneInterface::Path& dst, const pxr::SdfPath& src)
{
	SceneInterface::stringToPath(src.GetString(), dst);
}

void convertPath( pxr::SdfPath& dst, const SceneInterface::Path& src, bool relative = false)
{
	std::string pathAsString;
	SceneInterface::pathToString(src, pathAsString);
	if ( relative )
	{
		pathAsString.erase(0, 1);
	}

	dst = pxr::SdfPath( pathAsString );
}

IECoreScene::PointsPrimitivePtr convertPrimitive( pxr::UsdGeomPoints points, pxr::UsdTimeCode time )
{
	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive();
	PrimitiveAlgo::readPrimitiveVariables( points, time, newPoints.get() );
	if( auto *p = newPoints->variableData<V3fVectorData>( "P" ) )
	{
		newPoints->setNumPoints( p->readable().size() );
	}

	if( auto i = boost::static_pointer_cast<Int64VectorData>( DataAlgo::fromUSD( points.GetIdsAttr(), time ) ) )
	{
		newPoints->variables["id"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, i );
	}

	if( auto w = boost::static_pointer_cast<FloatVectorData>( DataAlgo::fromUSD( points.GetWidthsAttr(), time ) ) )
	{
		IECoreScene::PrimitiveVariable pv( PrimitiveAlgo::fromUSD( points.GetWidthsInterpolation() ), w );
		if( pv.interpolation == PrimitiveVariable::Constant && w->readable().size() == 1 )
		{
			// USD uses arrays even for constant data, but we use single values.
			pv.data = new FloatData( w->readable()[0] );
		}
		newPoints->variables["width"] = pv;
	}

	return newPoints;
}

IECoreScene::PointsPrimitivePtr convertPrimitive( pxr::UsdGeomPointInstancer pointInstancer, pxr::UsdTimeCode time )
{
	pxr::VtVec3fArray pointsData;
	pointInstancer.GetPositionsAttr().Get( &pointsData, time );
	IECore::V3fVectorDataPtr positionData = DataAlgo::fromUSD( pointsData );
	positionData->setInterpretation( GeometricData::Point );
	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive( positionData );

	// Per point attributes

	if( auto protoIndicesData = DataAlgo::fromUSD( pointInstancer.GetProtoIndicesAttr(), time ) )
	{
		newPoints->variables["prototypeIndex"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, protoIndicesData );
	}

	if( auto idsData = DataAlgo::fromUSD( pointInstancer.GetIdsAttr(), time ) )
	{
		newPoints->variables["instanceId"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, idsData );
	}

	if( auto orientationData = DataAlgo::fromUSD( pointInstancer.GetOrientationsAttr(), time ) )
	{
		newPoints->variables["orientation"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, orientationData );
	}

	if( auto scaleData = DataAlgo::fromUSD( pointInstancer.GetScalesAttr(), time ) )
	{
		newPoints->variables["scale"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, scaleData );
	}

	if( auto velocityData = DataAlgo::fromUSD( pointInstancer.GetVelocitiesAttr(), time ) )
	{
		newPoints->variables["velocity"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, velocityData );
	}

#if USD_VERSION >= 1911
	if( auto accelerationData = DataAlgo::fromUSD( pointInstancer.GetAccelerationsAttr(), time ) )
	{
		newPoints->variables["acceleration"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, accelerationData );
	}
#endif

	if( auto angularVelocityData = DataAlgo::fromUSD( pointInstancer.GetAngularVelocitiesAttr(), time ) )
	{
		newPoints->variables["angularVelocity"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, angularVelocityData );
	}

	// Prototype paths

	pxr::SdfPathVector targets;
	pointInstancer.GetPrototypesRel().GetTargets( &targets );

	IECore::StringVectorDataPtr prototypeRootsData = new IECore::StringVectorData();
	auto &prototypeRoots = prototypeRootsData->writable();
	prototypeRoots.reserve( targets.size() );
	for( const auto &t : targets )
	{
		prototypeRoots.push_back( t.GetString() );
	}

	newPoints->variables["prototypeRoots"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Constant, prototypeRootsData );

	return newPoints;
}

IECoreScene::CurvesPrimitivePtr convertPrimitive( pxr::UsdGeomBasisCurves curves, pxr::UsdTimeCode time )
{
	pxr::VtIntArray vertexCountsArray;
	curves.GetCurveVertexCountsAttr().Get( &vertexCountsArray, time );
	IECore::IntVectorDataPtr countData = DataAlgo::fromUSD( vertexCountsArray );

	// Basis

	IECore::CubicBasisf basis = CubicBasisf::linear();
	pxr::TfToken type;
	curves.GetTypeAttr().Get( &type, time );
	if( type == pxr::UsdGeomTokens->cubic )
	{
		pxr::TfToken usdBasis;
		curves.GetBasisAttr().Get( &usdBasis, time );
		if( usdBasis == pxr::UsdGeomTokens->bezier )
		{
			basis = CubicBasisf::bezier();
		}
		else if( usdBasis == pxr::UsdGeomTokens->bspline )
		{
			basis = CubicBasisf::bSpline();
		}
		else if( usdBasis == pxr::UsdGeomTokens->catmullRom )
		{
			basis = CubicBasisf::catmullRom();
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "USDScene", boost::format( "Unsupported basis \"%1%\"" ) % usdBasis );
		}
	}

	// Wrap

	bool periodic = false;
	pxr::TfToken wrap;
	curves.GetWrapAttr().Get( &wrap, time );
	if( wrap == pxr::UsdGeomTokens->periodic )
	{
		periodic = true;
	}
	else if( wrap != pxr::UsdGeomTokens->nonperiodic )
	{
		IECore::msg( IECore::Msg::Warning, "USDScene", boost::format( "Unsupported wrap \"%1%\"" ) % wrap );
	}

	// Curves and primvars

	IECoreScene::CurvesPrimitivePtr newCurves = new IECoreScene::CurvesPrimitive( countData, basis, periodic );
	PrimitiveAlgo::readPrimitiveVariables( curves, time, newCurves.get() );

	if( auto w = boost::static_pointer_cast<FloatVectorData>( DataAlgo::fromUSD( curves.GetWidthsAttr(), time ) ) )
	{
		IECoreScene::PrimitiveVariable pv( PrimitiveAlgo::fromUSD( curves.GetWidthsInterpolation() ), w );
		if( pv.interpolation == PrimitiveVariable::Constant && w->readable().size() == 1 )
		{
			// USD uses arrays even for constant data, but we use single values.
			pv.data = new FloatData( w->readable()[0] );
		}
		newCurves->variables["width"] = pv;
	}

	return newCurves;
}

IECoreScene::MeshPrimitivePtr convertPrimitive( pxr::UsdGeomMesh mesh, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute subdivSchemeAttr = mesh.GetSubdivisionSchemeAttr();

	pxr::TfToken subdivScheme;
	subdivSchemeAttr.Get( &subdivScheme );

	pxr::VtIntArray faceVertexCounts;
	mesh.GetFaceVertexCountsAttr().Get( &faceVertexCounts, time );
	IECore::IntVectorDataPtr vertexCountData = DataAlgo::fromUSD( faceVertexCounts );

	pxr::VtIntArray faceVertexIndices;
	mesh.GetFaceVertexIndicesAttr().Get( &faceVertexIndices, time  );
	IECore::IntVectorDataPtr vertexIndicesData = DataAlgo::fromUSD( faceVertexIndices );

	IECoreScene::MeshPrimitivePtr newMesh = new IECoreScene::MeshPrimitive( vertexCountData, vertexIndicesData );
	PrimitiveAlgo::readPrimitiveVariables( mesh, time, newMesh.get() );

	if( subdivScheme == pxr::UsdGeomTokens->catmullClark )
	{
		newMesh->setInterpolation( "catmullClark" );
	}

	// Corners

	pxr::VtIntArray cornerIndices;
	pxr::VtFloatArray cornerSharpnesses;
	mesh.GetCornerIndicesAttr().Get( &cornerIndices, time );
	mesh.GetCornerSharpnessesAttr().Get( &cornerSharpnesses, time );
	if( cornerIndices.size() )
	{
		IECore::IntVectorDataPtr cornerIndicesData = DataAlgo::fromUSD( cornerIndices );
		IECore::FloatVectorDataPtr cornerSharpnessesData = DataAlgo::fromUSD( cornerSharpnesses );
		newMesh->setCorners( cornerIndicesData.get(), cornerSharpnessesData.get() );
	}

	// Creases

	pxr::VtIntArray creaseLengths;
	pxr::VtIntArray creaseIndices;
	pxr::VtFloatArray creaseSharpnesses;
	mesh.GetCreaseLengthsAttr().Get( &creaseLengths, time );
	mesh.GetCreaseIndicesAttr().Get( &creaseIndices, time );
	mesh.GetCreaseSharpnessesAttr().Get( &creaseSharpnesses, time );
	if( creaseLengths.size() )
	{
		if( creaseSharpnesses.size() == creaseLengths.size() )
		{
			IECore::IntVectorDataPtr creaseLengthsData = DataAlgo::fromUSD( creaseLengths );
			IECore::IntVectorDataPtr creaseIndicesData = DataAlgo::fromUSD( creaseIndices );
			IECore::FloatVectorDataPtr creaseSharpnessesData = DataAlgo::fromUSD( creaseSharpnesses );
			newMesh->setCreases( creaseLengthsData.get(), creaseIndicesData.get(), creaseSharpnessesData.get() );
		}
		else
		{
			// USD documentation suggests that it is possible to author a sharpness per edge
			// within a single crease, rather than just a sharpness per crease. We don't know how
			// we would author one of these in practice (certainly not in Maya), and we're not sure
			// why we'd want to. For now we ignore them.
			IECore::msg( IECore::Msg::Warning, "USDScene", "Ignoring creases with varying sharpness" );
		}
	}

	return newMesh;
}

IECoreScene::SpherePrimitivePtr convertPrimitive( pxr::UsdGeomSphere sphere, pxr::UsdTimeCode time )
{
	double radius = 1.0f;
	sphere.GetRadiusAttr().Get( &radius, time );
	IECoreScene::SpherePrimitivePtr newSphere = new IECoreScene::SpherePrimitive( (float) radius );
	PrimitiveAlgo::readPrimitiveVariables( pxr::UsdGeomPrimvarsAPI( sphere.GetPrim() ), time, newSphere.get() );
	return newSphere;
}

void convertCamera( pxr::UsdGeomCamera usdCamera, const IECoreScene::Camera *camera, pxr::UsdTimeCode timeCode )
{
	if( camera->getProjection() == "orthographic" )
	{
		usdCamera.GetProjectionAttr().Set( pxr::TfToken( "orthographic" ) );

		// For ortho cameras, USD uses aperture units of tenths of scene units
		usdCamera.GetHorizontalApertureAttr().Set( 10.0f * camera->getAperture()[0] );
		usdCamera.GetVerticalApertureAttr().Set( 10.0f * camera->getAperture()[1] );
		usdCamera.GetHorizontalApertureOffsetAttr().Set( 10.0f * camera->getApertureOffset()[0] );
		usdCamera.GetVerticalApertureOffsetAttr().Set( 10.0f * camera->getApertureOffset()[1] );
	}
	else if( camera->getProjection() == "perspective" )
	{
		usdCamera.GetProjectionAttr().Set( pxr::TfToken( "perspective" ) );

		// We store focalLength and aperture in arbitary units.  USD uses tenths
		// of scene units
		float scale = 10.0f * camera->getFocalLengthWorldScale();

		usdCamera.GetFocalLengthAttr().Set( camera->getFocalLength() * scale );
		usdCamera.GetHorizontalApertureAttr().Set( camera->getAperture()[0] * scale );
		usdCamera.GetVerticalApertureAttr().Set( camera->getAperture()[1] * scale );
		usdCamera.GetHorizontalApertureOffsetAttr().Set( camera->getApertureOffset()[0] * scale );
		usdCamera.GetVerticalApertureOffsetAttr().Set( camera->getApertureOffset()[1] * scale );
	}
	else
	{
		// TODO - should we throw an error if you try to convert an unsupported projection type?
		return;
	}

	usdCamera.GetClippingRangeAttr().Set( pxr::GfVec2f( camera->getClippingPlanes().getValue() ) );
	usdCamera.GetFStopAttr().Set( camera->getFStop() );
	usdCamera.GetFocusDistanceAttr().Set( camera->getFocusDistance() );
	usdCamera.GetShutterOpenAttr().Set( (double)camera->getShutter()[0] );
	usdCamera.GetShutterCloseAttr().Set( (double)camera->getShutter()[1] );
}

void convertPrimitive( pxr::UsdGeomMesh usdMesh, const IECoreScene::MeshPrimitive *mesh, pxr::UsdTimeCode timeCode )
{
	// convert topology
	usdMesh.CreateFaceVertexCountsAttr().Set( DataAlgo::toUSD( mesh->verticesPerFace() ), timeCode );
	usdMesh.CreateFaceVertexIndicesAttr().Set( DataAlgo::toUSD( mesh->vertexIds() ), timeCode );

	// set the interpolation

	if (mesh->interpolation() == std::string("catmullClark"))
	{
		usdMesh.CreateSubdivisionSchemeAttr().Set( pxr::UsdGeomTokens->catmullClark );
	}
	else
	{
		usdMesh.CreateSubdivisionSchemeAttr().Set( pxr::UsdGeomTokens->none );
	}

	// corners

	if( mesh->cornerIds()->readable().size() )
	{
		usdMesh.CreateCornerIndicesAttr().Set( DataAlgo::toUSD( mesh->cornerIds() ) );
		usdMesh.CreateCornerSharpnessesAttr().Set( DataAlgo::toUSD( mesh->cornerSharpnesses() ) );
	}

	// creases

	if( mesh->creaseLengths()->readable().size() )
	{
		usdMesh.CreateCreaseLengthsAttr().Set( DataAlgo::toUSD( mesh->creaseLengths() ) );
		usdMesh.CreateCreaseIndicesAttr().Set( DataAlgo::toUSD( mesh->creaseIds() ) );
		usdMesh.CreateCreaseSharpnessesAttr().Set( DataAlgo::toUSD(  mesh->creaseSharpnesses() ) );
	}

	// convert all primvars to USD

	for( const auto &p : mesh->variables )
	{
		PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdMesh, timeCode );
	}
}

void convertPrimitive( pxr::UsdGeomPoints usdPoints, const IECoreScene::PointsPrimitive *points, pxr::UsdTimeCode timeCode )
{
	for( const auto &p : points->variables )
	{
		if( p.first == "id" )
		{
			usdPoints.CreateIdsAttr().Set( DataAlgo::toUSD( p.second.data.get() ), timeCode );
		}
		else if( p.first == "width" )
		{
			auto widthsAttr = usdPoints.CreateWidthsAttr();
			auto floatData = runTimeCast<const FloatData>( p.second.data.get() );
			if( p.second.interpolation == PrimitiveVariable::Constant && floatData )
			{
				// USD requires an array even for constant data.
				widthsAttr.Set( pxr::VtArray<float>( 1, floatData->readable() ), timeCode );
			}
			else
			{
				widthsAttr.Set( PrimitiveAlgo::toUSDExpanded( p.second ), timeCode );
			}
			usdPoints.SetWidthsInterpolation( PrimitiveAlgo::toUSD( p.second.interpolation ) );
		}
		else
		{
			PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdPoints, timeCode );
		}
	}
}

void convertPrimitive( pxr::UsdGeomBasisCurves usdCurves, const IECoreScene::CurvesPrimitive *curves, pxr::UsdTimeCode timeCode )
{
	// Topology, wrap, basis

	usdCurves.CreateCurveVertexCountsAttr().Set( DataAlgo::toUSD( curves->verticesPerCurve() ), timeCode );

	usdCurves.CreateWrapAttr().Set(
		curves->periodic() ? pxr::UsdGeomTokens->periodic : pxr::UsdGeomTokens->nonperiodic,
		timeCode
	);

	pxr::TfToken basis;
	if( curves->basis() == CubicBasisf::bezier() )
	{
		basis = pxr::UsdGeomTokens->bezier;
	}
	else if( curves->basis() == CubicBasisf::bSpline() )
	{
		basis = pxr::UsdGeomTokens->bspline;
	}
	else if( curves->basis() == CubicBasisf::catmullRom() )
	{
		basis = pxr::UsdGeomTokens->catmullRom;
	}
	else if ( curves->basis() != CubicBasisf::linear() )
	{
		IECore::msg( IECore::Msg::Warning, "USDScene", "Unsupported basis" );
	}

	if( !basis.IsEmpty() )
	{
		usdCurves.CreateTypeAttr().Set( pxr::UsdGeomTokens->cubic, timeCode );
		usdCurves.CreateBasisAttr().Set( basis, timeCode );
	}
	else
	{
		usdCurves.CreateTypeAttr().Set( pxr::UsdGeomTokens->linear, timeCode );
	}

	// Primvars

	for( const auto &p : curves->variables )
	{
		if( p.first == "width" )
		{
			auto widthsAttr = usdCurves.CreateWidthsAttr();
			auto floatData = runTimeCast<const FloatData>( p.second.data.get() );
			if( p.second.interpolation == PrimitiveVariable::Constant && floatData )
			{
				// USD requires an array even for constant data.
				widthsAttr.Set( pxr::VtArray<float>( 1, floatData->readable() ), timeCode );
			}
			else
			{
				widthsAttr.Set( PrimitiveAlgo::toUSDExpanded( p.second ), timeCode );
			}
			usdCurves.SetWidthsInterpolation( PrimitiveAlgo::toUSD( p.second.interpolation ) );
		}
		else
		{
			PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdCurves, timeCode );
		}
	}
}

void convertPrimitive( pxr::UsdGeomSphere usdSphere, const IECoreScene::SpherePrimitive *sphere, pxr::UsdTimeCode timeCode )
{
	// todo what should we do here if we loose SpherePrimitive information
	// writing out to USD?
	usdSphere.CreateRadiusAttr().Set( (double) sphere->radius() );
	for( const auto &p : sphere->variables )
	{
		PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, pxr::UsdGeomPrimvarsAPI( usdSphere.GetPrim() ), timeCode );
	}
}

bool isConvertible( pxr::UsdPrim prim )
{
	pxr::UsdGeomMesh mesh( prim );
	if( mesh )
	{
		return true;
	}

	pxr::UsdGeomPoints points( prim );
	if( points )
	{
		return true;
	}

	pxr::UsdGeomPointInstancer pointInstancer ( prim );
	if ( pointInstancer )
	{
		return true;
	}

	pxr::UsdGeomBasisCurves curves( prim );
	if( curves )
	{
		return true;
	}

	pxr::UsdGeomSphere sphere( prim );
	if ( sphere )
	{
		return true;
	}

	return false;
}

IECore::ConstObjectPtr convertPrimitive( pxr::UsdPrim prim, pxr::UsdTimeCode time )
{
	if( pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh( prim ) )
	{
		return convertPrimitive( mesh, time );
	}

	if( pxr::UsdGeomPoints points = pxr::UsdGeomPoints( prim ) )
	{
		return convertPrimitive( points, time );
	}

	if( pxr::UsdGeomPointInstancer pointInstancer = pxr::UsdGeomPointInstancer( prim ) )
	{
		return convertPrimitive( pointInstancer, time );
	}

	if( pxr::UsdGeomBasisCurves curves = pxr::UsdGeomBasisCurves( prim ) )
	{
		return convertPrimitive( curves, time );
	}

	if ( pxr::UsdGeomSphere sphere = pxr::UsdGeomSphere( prim ) )
	{
		return convertPrimitive( sphere, time );
	}

	return nullptr;
}

bool isTimeVarying( pxr::UsdGeomMesh mesh )
{
	return
		mesh.GetSubdivisionSchemeAttr().ValueMightBeTimeVarying() ||
		mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying() ||
		mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCornerIndicesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCornerSharpnessesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCreaseLengthsAttr().ValueMightBeTimeVarying() ||
		mesh.GetCreaseIndicesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCreaseSharpnessesAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( mesh )
	;
}

bool isTimeVarying( pxr::UsdGeomPointInstancer instancer )
{
	return
		instancer.GetPositionsAttr().ValueMightBeTimeVarying() ||
		instancer.GetProtoIndicesAttr().ValueMightBeTimeVarying() ||
		instancer.GetIdsAttr().ValueMightBeTimeVarying() ||
		instancer.GetOrientationsAttr().ValueMightBeTimeVarying() ||
		instancer.GetScalesAttr().ValueMightBeTimeVarying() ||
		instancer.GetVelocitiesAttr().ValueMightBeTimeVarying() ||
#if USD_VERSION >= 1911
		instancer.GetAccelerationsAttr().ValueMightBeTimeVarying() ||
#endif
		instancer.GetAngularVelocitiesAttr().ValueMightBeTimeVarying()
	;
}

bool isTimeVarying( pxr::UsdGeomBasisCurves curves )
{
	return
		curves.GetCurveVertexCountsAttr().ValueMightBeTimeVarying() ||
		curves.GetTypeAttr().ValueMightBeTimeVarying() ||
		curves.GetBasisAttr().ValueMightBeTimeVarying() ||
		curves.GetWrapAttr().ValueMightBeTimeVarying() ||
		curves.GetWidthsAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( curves )
	;
}

bool isTimeVarying( pxr::UsdGeomPoints points )
{
	return
		points.GetIdsAttr().ValueMightBeTimeVarying() ||
		points.GetWidthsAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( points )
	;
}

bool isTimeVarying( pxr::UsdGeomSphere sphere )
{
	return
		sphere.GetRadiusAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( pxr::UsdGeomPrimvarsAPI( sphere.GetPrim() ) )
	;
}

bool isTimeVarying( pxr::UsdPrim prim )
{
	if( pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh( prim ) )
	{
		return isTimeVarying( mesh );
	}

	if( pxr::UsdGeomPoints points = pxr::UsdGeomPoints ( prim ) )
	{
		return isTimeVarying( points );
	}

	if( pxr::UsdGeomPointInstancer pointInstancer = pxr::UsdGeomPointInstancer( prim ) )
	{
		return isTimeVarying( pointInstancer );
	}

	if( pxr::UsdGeomBasisCurves curves = pxr::UsdGeomBasisCurves( prim ) )
	{
		return isTimeVarying( curves );
	}

	if ( pxr::UsdGeomSphere sphere = pxr::UsdGeomSphere( prim ) )
	{
		return isTimeVarying( sphere );
	}

	return false;
}

SceneInterface::Name convertAttributeName(const pxr::TfToken& attributeName)
{
	return SceneInterface::Name ( boost::algorithm::replace_first_copy( attributeName.GetString(), "cortex:", "" ) );
}

pxr::TfToken convertAttributeName(const SceneInterface::Name &attributeName)
{
	return pxr::TfToken( std::string("cortex:") +  attributeName.string() );
}

bool isAttributeName( const pxr::TfToken& attributeName )
{
	return boost::algorithm::starts_with( attributeName.GetString(), "cortex:" );
}

pxr::TfToken validName( const std::string &name )
{
	// `TfMakeValidIdentifier` _almost_ does what we want, but in Gaffer
	// we use purely numeric identifiers for instance names, and
	// `TfMakeValidIdentifier` replaces leading non-alphanumeric characters
	// with '_', meaning that `0-9` all become `_`. We want to _prefix_ with
	// an `_` instead to preserve uniqueness.

	if( name.size() && '0' <= name[0] && name[0] <= '9' )
	{
		return pxr::TfToken( pxr::TfMakeValidIdentifier( "_" + name ) );
	}
	else
	{
		return pxr::TfToken( pxr::TfMakeValidIdentifier( name ) );
	}
}

} // namespace

class USDScene::Location : public RefCounted
{
	public:
		Location(pxr::UsdPrim prim ) : prim(prim) {}
		pxr::UsdPrim prim;
};

class USDScene::IO : public RefCounted
{
	public:
		IO( const std::string &fileName ) : m_fileName( fileName )
		{
		}

		virtual ~IO()
		{
		}

		const std::string &fileName() const
		{
			return m_fileName;
		}

		virtual pxr::UsdPrim root() const = 0;
		virtual pxr::UsdTimeCode getTime( double timeSeconds ) const = 0;

		virtual bool isReader() const = 0;

		pxr::UsdStageRefPtr getStage() const { return m_usdStage; }
	protected:
		pxr::UsdStageRefPtr m_usdStage;

	private:
		std::string m_fileName;
};

class USDScene::Reader : public USDScene::IO
{
	public:
		Reader( const std::string &fileName ) : IO( fileName )
		{
			m_usdStage = pxr::UsdStage::Open( fileName );

			if ( !m_usdStage )
			{
				throw IECore::Exception( boost::str( boost::format( "USDScene::Reader() Failed to open usd file: '%1%'" ) % fileName ) );
			}

			m_timeCodesPerSecond = m_usdStage->GetTimeCodesPerSecond();
			m_rootPrim = m_usdStage->GetPseudoRoot();
		}

		pxr::UsdPrim root() const override
		{
			return m_rootPrim;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const override
		{
			return timeSeconds * m_timeCodesPerSecond;
		}

		bool isReader()  const override { return true; }

	private:

		pxr::UsdPrim m_rootPrim;

		double m_timeCodesPerSecond;
};

class USDScene::Writer : public USDScene::IO
{
	public:
		Writer( const std::string &fileName ) : IO( fileName )
		{
			m_usdStage = pxr::UsdStage::CreateNew( fileName );
			m_timeCodesPerSecond = m_usdStage->GetTimeCodesPerSecond();
			m_rootPrim = m_usdStage->GetPseudoRoot();
		}

		~Writer() override
		{
			m_usdStage->GetRootLayer()->Save();
		}

		pxr::UsdPrim root() const override
		{
			return m_rootPrim;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const override
		{
			return timeSeconds * m_timeCodesPerSecond;
		}

		bool isReader()  const override { return false; }

	private:

		pxr::UsdPrim m_rootPrim;

		double m_timeCodesPerSecond;
};

USDScene::USDScene( const std::string &path, IndexedIO::OpenMode &mode )
{
	switch( mode )
	{
		case IndexedIO::Read :
			m_root = new Reader( path );
			m_location = new Location( m_root->root() );
			break;
		case IndexedIO::Write :
			m_root = new Writer( path );
			m_location = new Location( m_root->root() );
			break;
		default:
			throw Exception( " Unsupported OpenMode " );
	}
}

USDScene::USDScene( IOPtr root, LocationPtr location) : m_root( root ), m_location( location )
{

}

USDScene::~USDScene()
{
}

std::string USDScene::fileName() const
{
	return m_root->fileName();
}

Imath::Box3d USDScene::readBound( double time ) const
{
	pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim );
	pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh ( m_location->prim );

	if( !boundable )
	{
		return Imath::Box3d();
	}

	pxr::UsdAttribute attr = boundable.GetExtentAttr();

	if( !attr.IsValid() )
	{
		return Imath::Box3d();
	}

	pxr::VtArray<pxr::GfVec3f> extents;
	attr.Get<pxr::VtArray<pxr::GfVec3f> >( &extents, m_root->getTime( time ) );

	if( extents.size() == 2 )
	{
		return Imath::Box3d(
			DataAlgo::fromUSD( extents[0] ),
			DataAlgo::fromUSD( extents[1] )
		);
	}

	return Imath::Box3d();
}

ConstDataPtr USDScene::readTransform( double time ) const
{
	return new IECore::M44dData( readTransformAsMatrix( time ) );
}

Imath::M44d USDScene::readTransformAsMatrix( double time ) const
{
	bool zUp = m_location->prim.GetParent().IsPseudoRoot() && pxr::UsdGeomGetStageUpAxis( m_root->getStage() ) == pxr::UsdGeomTokens->z;

	pxr::UsdGeomXformable transformable( m_location->prim );
	pxr::GfMatrix4d transform;
	bool reset = false;

	transformable.GetLocalTransformation( &transform, &reset, m_root->getTime( time ) );
	Imath::M44d returnValue = DataAlgo::fromUSD( transform );

	if ( zUp )
	{
		static Imath::M44d b
			(
				0, 0, 1, 0,
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 0, 1
			);

		returnValue = returnValue * b;
	}
	return returnValue;
}

ConstObjectPtr USDScene::readAttribute( const SceneInterface::Name &name, double time ) const
{
	pxr::UsdAttribute attribute = m_location->prim.GetAttribute( convertAttributeName( name ) );

	if ( !attribute )
	{
		return nullptr;
	}

	pxr::VtValue value;
	if ( !attribute.Get(&value, m_root->getTime( time ) ) )
	{
		return nullptr;
	}

	DataPtr data = DataAlgo::fromUSD( value, attribute.GetTypeName() );
	if( !data )
	{
		IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format( "Unknown type %1% on attribute %2%") % attribute.GetTypeName() % name.string());
	}

	return data;
}

ConstObjectPtr USDScene::readObject( double time ) const
{
	return convertPrimitive( m_location->prim, m_root->getTime( time ) );
}

SceneInterface::Name USDScene::name() const
{
	return SceneInterface::Name( m_location->prim.GetName() );
}

void USDScene::path( SceneInterface::Path &p ) const
{
	std::vector<std::string> parts;
	pxr::SdfPath path = m_location->prim.GetPath();
	boost::split( parts, path.GetString(), boost::is_any_of( "/" ) );

	p.reserve( parts.size() );

	for( const auto &part : parts )
	{
		if( part != "" )
		{
			p.push_back( IECore::InternedString( part ) );
		}
	}
}

bool USDScene::hasBound() const
{
	pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim );
	pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh( m_location->prim );
	pxr::UsdAttribute attr;

	if( boundable )
	{
		attr = boundable.GetExtentAttr();
	}

	return attr.IsValid();
}

void USDScene::writeBound( const Imath::Box3d &bound, double time )
{
	// unable to write bounds on root scene graph location
	if( m_location->prim.GetPath().IsEmpty() )
	{
		return;
	}

	pxr::UsdGeomBoundable boundable( m_location->prim );
	if( !boundable )
	{
		return;
	}

	pxr::VtArray<pxr::GfVec3f> extent;
	extent.push_back( DataAlgo::toUSD( Imath::V3f( bound.min ) ) );
	extent.push_back( DataAlgo::toUSD( Imath::V3f( bound.max ) ) );

	pxr::UsdAttribute extentAttr = boundable.CreateExtentAttr();
	extentAttr.Set( pxr::VtValue( extent ) );
}

void USDScene::writeTransform( const Data *transform, double time )
{
	const M44dData *m44 = IECore::runTimeCast<const M44dData>( transform );
	if( !m44 )
	{
		return;
	}

	pxr::UsdGeomXformable xformable( m_location->prim );
	if( xformable )
	{
		pxr::UsdGeomXformOp transformOp = xformable.MakeMatrixXform();
		const pxr::UsdTimeCode timeCode = m_root->getTime( time );
		transformOp.Set( DataAlgo::toUSD( m44->readable() ), timeCode );
	}
}

bool USDScene::hasAttribute( const SceneInterface::Name &name ) const
{
	return m_location->prim.HasAttribute( convertAttributeName( name ) );
}

void USDScene::attributeNames( SceneInterface::NameList &attrs ) const
{
	std::vector<pxr::UsdAttribute> attributes = m_location->prim.GetAttributes();

	attrs.clear();
	attrs.reserve( attributes.size() );

	for( const auto &attr : attributes )
	{
		if ( isAttributeName( attr.GetName() ) )
		{
			attrs.push_back( convertAttributeName ( attr.GetName() ) );
		}
	}
}

void USDScene::writeAttribute( const SceneInterface::Name &name, const Object *attribute, double time )
{
	if( auto *data = IECore::runTimeCast<const IECore::Data>( attribute ) )
	{
		const pxr::UsdTimeCode timeCode = m_root->getTime( time );
		pxr::UsdAttribute attribute = m_location->prim.CreateAttribute( convertAttributeName( name ), DataAlgo::valueTypeName( data ), true );
		attribute.Set( DataAlgo::toUSD( data ), timeCode );
	}
}

bool USDScene::hasTag( const SceneInterface::Name &name, int filter ) const
{
	pxr::UsdPrim defaultPrim = m_root->getStage()->GetDefaultPrim();
	if ( !defaultPrim )
	{
		return false;
	}

	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI( defaultPrim, pxr::TfToken( name.string() ) );
	if (!collection)
	{
		return false;
	}

	pxr::SdfPath p = m_location->prim.GetPath();

	pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();
	pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths(membershipQuery, m_root->getStage());

	/// TODO. This will need to be updated once the Gaffer path matcher functionality has been moved into cortex
	for ( const auto &path : includedPaths )
	{
		if (path == p && filter & SceneInterface::LocalTag)
		{
			return true;
		}

		if (filter & SceneInterface::DescendantTag && boost::algorithm::starts_with( path.GetString(), p.GetString() ) && path != p )
		{
			return true;
		}

		if (filter & SceneInterface::AncestorTag && boost::algorithm::starts_with( p.GetString(), path.GetString() ) && path != p)
		{
			return true;
		}
	}

	return false;
}

void USDScene::readTags( SceneInterface::NameList &tags, int filter ) const
{
	pxr::UsdPrim defaultPrim = m_root->getStage()->GetDefaultPrim();

	if ( !defaultPrim )
	{
		return;
	}

	tags.clear();
	std::vector<pxr::UsdCollectionAPI> collectionAPIs = pxr::UsdCollectionAPI::GetAllCollections( defaultPrim );
	pxr::SdfPath currentPath = m_location->prim.GetPath();

	pxr::SdfPath p = m_location->prim.GetPath();

	/// TODO. This will need to be updated once the Gaffer path matcher functionality has been moved into cortex
	std::set<SceneInterface::Name> tagsSet;
	for ( const auto &collection : collectionAPIs)
	{
		pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();
		pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths(membershipQuery, m_root->getStage());

		for ( const auto &path : includedPaths )
		{
			bool match = false;
			if (path == p && filter & SceneInterface::LocalTag)
			{
				match = true;
			}

			if (filter & SceneInterface::DescendantTag && boost::algorithm::starts_with( path.GetString(), p.GetString() ) && path != p )
			{
				match = true;
			}

			if (filter & SceneInterface::AncestorTag && boost::algorithm::starts_with( p.GetString(), path.GetString() ) && path != p )
			{
				match = true;
			}

			if ( match )
			{
				tagsSet.insert( collection.GetName().GetString() );
			}
		}
	}

	for (const auto& i : tagsSet)
	{
		tags.push_back( i );
	}

}

void USDScene::writeTags( const SceneInterface::NameList &tags )
{
	pxr::UsdPrim defaultPrim = m_root->getStage()->GetDefaultPrim();

	if ( !defaultPrim )
	{
		defaultPrim = m_root->getStage()->DefinePrim( pxr::SdfPath( "/sets" ) );
		m_root->getStage()->SetDefaultPrim( defaultPrim );
	}

	for( const auto &tag : tags )
	{
		pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI::ApplyCollection( defaultPrim, pxr::TfToken( tag.string() ), pxr::UsdTokens->explicitOnly );
		collection.CreateIncludesRel().AddTarget( m_location->prim.GetPath() );
	}
}

SceneInterface::NameList USDScene::setNames( bool includeDescendantSets ) const
{
	std::vector<pxr::UsdCollectionAPI> allCollections = pxr::UsdCollectionAPI::GetAllCollections( m_location->prim );
	NameList setNames;

	setNames.reserve( allCollections.size() );
	for( const pxr::UsdCollectionAPI &collection : allCollections )
	{
		setNames.push_back( collection.GetName().GetString() );
	}

	if ( includeDescendantSets )
	{
		NameList children;
		childNames( children );
		for( const SceneInterface::Name &childName : children )
		{
			NameList childSetNames = child( childName, ThrowIfMissing )->setNames( includeDescendantSets );
			setNames.insert( setNames.begin(), childSetNames.begin(), childSetNames.end() );
		}
	}

	// ensure our set names are unique
	std::sort( setNames.begin(), setNames.end() );
	return NameList( setNames.begin(), std::unique( setNames.begin(), setNames.end() ) );
}

PathMatcher USDScene::readSet( const Name &name, bool includeDescendantSets ) const
{
	SceneInterface::Path prefix;
	PathMatcher pathMatcher;
	recurseReadSet( prefix, name, pathMatcher, includeDescendantSets );

	return pathMatcher;
}

void USDScene::recurseReadSet( const SceneInterface::Path &prefix, const Name &name, IECore::PathMatcher &pathMatcher, bool includeDescendantSets ) const
{
	if( PathMatcherDataPtr pathMatcherData = readLocalSet( name ) )
	{
		pathMatcher.addPaths( pathMatcherData->readable(), prefix );
	}

	if ( !includeDescendantSets )
	{
		return;
	}

	NameList children;
	childNames( children );

	SceneInterface::Path childPrefix = prefix;
	childPrefix.resize( prefix.size() + 1 );

	for( InternedString &childName : children )
	{
		*childPrefix.rbegin() = childName;
		runTimeCast<const USDScene>( child( childName, SceneInterface::ThrowIfMissing ) )->recurseReadSet( childPrefix, name, pathMatcher, includeDescendantSets );
	}
}

IECore::PathMatcherDataPtr USDScene::readLocalSet( const Name &name ) const
{
	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI( m_location->prim, pxr::TfToken( name.string() ) );

	if( !collection )
	{
		return new IECore::PathMatcherData();
	}

	pxr::UsdCollectionAPI::MembershipQuery membershipQuery = collection.ComputeMembershipQuery();
	pxr::SdfPathSet includedPaths = collection.ComputeIncludedPaths( membershipQuery, m_root->getStage() );

	PathMatcherDataPtr pathMatcherData = new PathMatcherData();
	PathMatcher &pathMatcher = pathMatcherData->writable();

	for( pxr::SdfPath path : includedPaths )
	{
		path = path.ReplacePrefix( m_location->prim.GetPath(), pxr::SdfPath( "/" ) );

		SceneInterface::Path cortexPath;
		convertPath( cortexPath, path );

		pathMatcher.addPath( cortexPath );
	}

	return pathMatcherData;
}

void USDScene::writeSet( const Name &name, const IECore::PathMatcher &set )
{
	pxr::UsdCollectionAPI collection = pxr::UsdCollectionAPI::ApplyCollection( m_location->prim, pxr::TfToken( name.string() ), pxr::UsdTokens->explicitOnly );

	for( PathMatcher::Iterator it = set.begin(); it != set.end(); ++it )
	{
		const SceneInterface::Path &path = *it;

		if ( path.empty() )
		{
			IECore::msg(
				IECore::MessageHandler::Error,
				"USDScene::writeSet",
				boost::str( boost::format( "Unable to add path '%2%' to  set: '%1%' at location: '%2%' " ) % name.string() % m_location->prim.GetPath().GetString() )
			);
			continue;
		}

		pxr::SdfPath pxrPath;
		convertPath( pxrPath, path, true );

		collection.CreateIncludesRel().AddTarget( pxrPath );
	}
}

void USDScene::hashSet( const Name &name, IECore::MurmurHash &h ) const
{
	SceneInterface::hashSet( name, h );

	SceneInterface::Path path;
	convertPath( path, m_location->prim.GetPath() );

	h.append( m_root->fileName() );
	h.append( &path[0], path.size() );
	h.append( name );
}

bool USDScene::hasObject() const
{
	return isConvertible( m_location->prim );
}

PrimitiveVariableMap USDScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	return PrimitiveVariableMap();
}

void USDScene::writeObject( const Object *object, double time )
{
	pxr::UsdTimeCode timeCode = m_root->getTime( time );

	const IECoreScene::MeshPrimitive* meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>( object );
	if ( meshPrimitive )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define( m_root->getStage(), p );
		convertPrimitive( usdMesh, meshPrimitive, timeCode );
	}

	const IECoreScene::PointsPrimitive* pointsPrimitive = IECore::runTimeCast<const IECoreScene::PointsPrimitive>( object );
	if ( pointsPrimitive )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomPoints usdPoints = pxr::UsdGeomPoints::Define( m_root->getStage(), p );
		convertPrimitive( usdPoints, pointsPrimitive, timeCode );
	}

	const IECoreScene::CurvesPrimitive* curvesPrimitive = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>( object );
	if ( curvesPrimitive )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomBasisCurves usdCurves = pxr::UsdGeomBasisCurves::Define( m_root->getStage(), p );
		convertPrimitive( usdCurves, curvesPrimitive, timeCode );
	}

	const IECoreScene::SpherePrimitive* spherePrimitive = IECore::runTimeCast<const IECoreScene::SpherePrimitive>( object );
	if ( spherePrimitive )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomSphere usdSphere = pxr::UsdGeomSphere::Define( m_root->getStage(), p );
		convertPrimitive( usdSphere, spherePrimitive, timeCode );
	}

	const IECoreScene::Camera* camera = IECore::runTimeCast<const IECoreScene::Camera>( object );
	if( camera )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomCamera usdCamera = pxr::UsdGeomCamera::Define( m_root->getStage(), p );
		convertCamera( usdCamera, camera, timeCode );
	}
}

bool USDScene::hasChild( const SceneInterface::Name &name ) const
{
	pxr::UsdPrim childPrim = m_location->prim.GetChild( pxr::TfToken( name.string() ) );

	return (bool)childPrim;
}

void USDScene::childNames( SceneInterface::NameList &childNames ) const
{
	for( const auto &i : m_location->prim.GetFilteredChildren( pxr::UsdTraverseInstanceProxies() ) )
	{
		pxr::UsdGeomXformable xformable ( i );

		if( xformable )
		{
			childNames.push_back( IECore::InternedString( i.GetName() ) );
		}
	}
}

SceneInterfacePtr USDScene::child( const SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour )
{
	pxr::UsdPrim childPrim;
	if( pxr::TfIsValidIdentifier( name.string() ) )
	{
		childPrim = m_location->prim.GetChild( pxr::TfToken( name.string() ) );
	}

	if( childPrim )
	{
		if( ( childPrim.GetTypeName() == "Xform" || isConvertible( childPrim ) ) )
		{
			SceneInterfacePtr newScene = new USDScene( m_root, new Location(childPrim) );
			return newScene;
		}
	}

	switch( missingBehaviour )
	{
		case SceneInterface::NullIfMissing :
			return nullptr;
		case SceneInterface::ThrowIfMissing :
			throw IOException( "Child \"" + name.string() + "\" does not exist" );
		case SceneInterface::CreateIfMissing :
		{
			if( m_root->isReader() )
			{
				throw InvalidArgumentException( "Child creation not supported" );
			}
			else
			{
				return createChild( name );
			}
		}
		default:
			return nullptr;
	}
}

ConstSceneInterfacePtr USDScene::child( const SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return const_cast<USDScene *>( this )->child( name, missingBehaviour );
}

SceneInterfacePtr USDScene::createChild( const SceneInterface::Name &name )
{
	pxr::UsdPrim prim = m_location->prim;
	pxr::SdfPath newPath = prim.GetPath().AppendChild( validName( name ) );
	pxr::UsdGeomXform newXform = pxr::UsdGeomXform::Define( m_root->getStage(), newPath );

	return new USDScene( m_root, new Location( newXform.GetPrim() ) );
}

SceneInterfacePtr USDScene::scene( const SceneInterface::Path &path, SceneInterface::MissingBehaviour missingBehaviour )
{
	pxr::UsdPrim prim = m_location->prim;

	for( const Name &name : path )
	{
		prim = prim.GetChild( pxr::TfToken( name ) );
	}
	return new USDScene( m_root, new Location( prim ) );
}

ConstSceneInterfacePtr USDScene::scene( const SceneInterface::Path &path, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return const_cast<USDScene *>( this )->scene( path, missingBehaviour );
}

void USDScene::hash( SceneInterface::HashType hashType, double time, MurmurHash &h ) const
{
	SceneInterface::hash( hashType, time, h );

	h.append( hashType );

	switch( hashType )
	{
		case SceneInterface::TransformHash:
			transformHash( time, h );
			break;
		case SceneInterface::AttributesHash:
			break;
		case SceneInterface::BoundHash:
			boundHash( time, h );
			break;
		case SceneInterface::ObjectHash:
			objectHash( time, h );
			break;
		case SceneInterface::ChildNamesHash:
			childNamesHash( time, h );
			break;
		case SceneInterface::HierarchyHash:
			hierarchyHash( time, h );
			break;
	}
}

void USDScene::boundHash( double time, IECore::MurmurHash &h ) const
{
	if( pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim ) )
	{
		h.append( m_location->prim.GetPath().GetString() );
		h.append( m_root->fileName() );

		if( boundable.GetExtentAttr().ValueMightBeTimeVarying() )
		{
			h.append( time );
		}
	}
}

void USDScene::transformHash( double time, IECore::MurmurHash &h ) const
{
	if( pxr::UsdGeomXformable xformable = pxr::UsdGeomXformable( m_location->prim ) )
	{
		h.append( m_location->prim.GetPath().GetString() );
		h.append( m_root->fileName() );

		if( xformable.TransformMightBeTimeVarying() )
		{
			h.append( time );
		}
	}
}

void USDScene::attributeHash ( double time, IECore::MurmurHash &h) const
{

}

void USDScene::objectHash( double time, IECore::MurmurHash &h ) const
{
	if( isConvertible( m_location->prim ) )
	{
		h.append( m_location->prim.GetPath().GetString() );
		h.append( m_root->fileName() );

		if( isTimeVarying( m_location->prim ) )
		{
			h.append( time );
		}
	}
}
void USDScene::childNamesHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_location->prim.GetPath().GetString() );
	h.append( m_root->fileName() );
}

void USDScene::hierarchyHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_location->prim.GetPath().GetString() );
	h.append( m_root->fileName() );
	h.append( time );
}

namespace
{

SceneInterface::FileFormatDescription<USDScene> g_descriptionUSD( ".usd", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDA( ".usda", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDC( ".usdc", IndexedIO::Read | IndexedIO::Write );

} // namespace
