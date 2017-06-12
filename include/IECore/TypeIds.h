//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IE_CORE_TYPEIDS_H
#define IE_CORE_TYPEIDS_H

namespace IECore
{

enum TypeId
{
	InvalidTypeId = 0,
	ObjectTypeId = 1,
	DataTypeId = 2,
	FloatVectorDataTypeId = 3,
	DoubleVectorDataTypeId = 4,
	IntVectorDataTypeId = 5,
	UIntVectorDataTypeId = 6,
	CharVectorDataTypeId = 7,
	UCharVectorDataTypeId = 8,
	V2fVectorDataTypeId = 9,
	V2dVectorDataTypeId = 10,
	V3fVectorDataTypeId = 11,
	V3dVectorDataTypeId = 12,
	Box3fVectorDataTypeId = 13,
	Box3dVectorDataTypeId = 14,
	M33fVectorDataTypeId = 15,
	M33dVectorDataTypeId = 16,
	M44fVectorDataTypeId = 17,
	M44dVectorDataTypeId = 18,
	QuatfVectorDataTypeId = 19,
	QuatdVectorDataTypeId = 20,
	StringVectorDataTypeId = 21,
	FloatDataTypeId = 22,
	DoubleDataTypeId = 23,
	IntDataTypeId = 24,
	LongDataTypeId = 25, /// Obsolete: LongData has been removed. The typeId remains for compatibility with old files, which now load as IntData
	UIntDataTypeId = 26,
	CharDataTypeId = 27,
	UCharDataTypeId = 28,
	StringDataTypeId = 29,
	LongVectorDataTypeId = 30, /// Obsolete: LongVectorData has been removed. The typeId remains for compatibility with old files, which now load as IntVectorData
	CompoundDataTypeId = 31,
	V2fDataTypeId = 32,
	V3fDataTypeId = 33,
	V2dDataTypeId = 34,
	V3dDataTypeId = 35,
	Box2fDataTypeId = 36,
	Box3fDataTypeId = 37,
	Box2dDataTypeId = 38,
	Box3dDataTypeId = 39,
	M44fDataTypeId = 40,
	M44dDataTypeId = 41,
	QuatfDataTypeId = 42,
	QuatdDataTypeId = 43,
	Color3fDataTypeId = 44,
	Color4fDataTypeId = 45,
	Color3dDataTypeId = 46,
	Color4dDataTypeId = 47,
	Color3fVectorDataTypeId = 48,
	Color4fVectorDataTypeId = 49,
	Color3dVectorDataTypeId = 50,
	Color4dVectorDataTypeId = 51,
	BlindDataHolderTypeId = 52,
	RenderableTypeId = 53,
	ParameterListTypeId = 54, // Obsolete
	CompoundObjectTypeId = 55,
	M33fDataTypeId = 56,
	M33dDataTypeId = 57,
	Box2fVectorDataTypeId = 58,
	Box2dVectorDataTypeId = 59,
	BoolDataTypeId = 60,
	PrimitiveTypeId = 61,
	PointsPrimitiveTypeId = 62,
	ImagePrimitiveTypeId = 63,
	Box2iDataTypeId = 64,
	HalfVectorDataTypeId = 65,
	V2iDataTypeId = 66,
	MeshPrimitiveTypeId = 67,
	ShaderTypeId = 68,
	RunTimeTypedTypeId = 69,
	ParameterTypeId = 70,
	CompoundParameterTypeId = 71,
	StringParameterTypeId = 72,
	ValidatedStringParameterTypeId = 73,
	FileNameParameterTypeId = 74,
	IntParameterTypeId = 75,
	FloatParameterTypeId = 76,
	DoubleParameterTypeId = 77,
	BoolParameterTypeId = 78,
	V2fParameterTypeId = 79,
	V3fParameterTypeId = 80,
	V2dParameterTypeId = 81,
	V3dParameterTypeId = 82,
	Color3fParameterTypeId = 83,
	Color4fParameterTypeId = 84,
	Box2iParameterTypeId = 85,
	Box2fParameterTypeId = 86,
	Box3fParameterTypeId = 87,
	Box2dParameterTypeId = 88,
	Box3dParameterTypeId = 89,
	M44fParameterTypeId = 90,
	M44dParameterTypeId = 91,
	IntVectorParameterTypeId = 92,
	FloatVectorParameterTypeId = 93,
	DoubleVectorParameterTypeId = 94,
	StringVectorParameterTypeId = 95,
	V2fVectorParameterTypeId = 96,
	V3fVectorParameterTypeId = 97,
	V2dVectorParameterTypeId = 98,
	V3dVectorParameterTypeId = 99,
	Box3fVectorParameterTypeId = 100,
	Box3dVectorParameterTypeId = 101,
	M33fVectorParameterTypeId = 102,
	M44fVectorParameterTypeId = 103,
	M33dVectorParameterTypeId = 104,
	M44dVectorParameterTypeId = 105,
	QuatfVectorParameterTypeId = 106,
	QuatdVectorParameterTypeId = 107,
	Color3fVectorParameterTypeId = 108,
	Color4fVectorParameterTypeId = 109,
	NullObjectTypeId = 110,
	ParameterisedTypeId = 111,
	OpTypeId = 112,
	ReaderTypeId = 113,
	WriterTypeId = 114,
	ImageReaderTypeId = 115,
	ImageWriterTypeId = 116,
	CINImageReaderTypeId = 117, // obsolete - available for reuse
	CINImageWriterTypeId = 118, // obsolete - available for reuse
	EXRImageReaderTypeId = 119, // obsolete - available for reuse
	EXRImageWriterTypeId = 120, // obsolete - available for reuse
	JPEGImageReaderTypeId = 121, // obsolete - available for reuse
	JPEGImageWriterTypeId = 122, // obsolete - available for reuse
	TIFFImageReaderTypeId = 123, // obsolete - available for reuse
	TIFFImageWriterTypeId = 124, // obsolete - available for reuse
	ObjectReaderTypeId = 125,
	ObjectWriterTypeId = 126,
	PDCParticleReaderTypeId = 127,
	PDCParticleWriterTypeId = 128,
	PathParameterTypeId = 129,
	DirNameParameterTypeId = 130,
	V3iDataTypeId = 131,
	RendererTypeId = 132,
	Box3iDataTypeId = 133,
	ObjectParameterTypeId = 134,
	ModifyOpTypeId = 135,
	ImageOpTypeId = 136,
	PrimitiveOpTypeId = 137,
	ProceduralTypeId = 138, // Obsolete
	Box3iParameterTypeId = 139,
	V2iParameterTypeId = 140,
	V3iParameterTypeId = 141,
	ParticleReaderTypeId = 142,
	ParticleWriterTypeId = 143,
	MotionPrimitiveTypeId = 144,
	DPXImageReaderTypeId = 145, // obsolete - available for reuse
	TransformTypeId = 146,
	MatrixTransformTypeId = 147,
	MotionTransformTypeId = 148,
	MatrixMotionTransformTypeId = 149,
	GroupTypeId = 150,
	AttributeStateTypeId = 151,
	VisibleRenderableTypeId = 152,
	StateRenderableTypeId = 153,
	OBJReaderTypeId = 154,
	TransformationMatrixfDataTypeId = 155,
	TransformationMatrixdDataTypeId = 156,
	PointNormalsOpTypeId = 157,
	PointDensitiesOpTypeId = 158,
	DPXImageWriterTypeId = 159, // obsolete - available for reuse
	BoolVectorDataTypeId = 160,
	VectorDataFilterOpTypeId = 161,
	RenderableParameterTypeId = 162,
	StateRenderableParameterTypeId = 163,
	AttributeStateParameterTypeId = 164,
	ShaderParameterTypeId = 165,
	TransformParameterTypeId = 166,
	MatrixMotionTransformParameterTypeId = 167,
	MatrixTransformParameterTypeId = 168,
	VisibleRenderableParameterTypeId = 169,
	GroupParameterTypeId = 170,
	MotionPrimitiveParameterTypeId = 171,
	PrimitiveParameterTypeId = 172,
	ImagePrimitiveParameterTypeId = 173,
	MeshPrimitiveParameterTypeId = 174,
	PointsPrimitiveParameterTypeId = 175,
	PreWorldRenderableTypeId = 176,
	CameraTypeId = 177,
	NURBSPrimitiveTypeId = 178,
	DataCastOpTypeId = 179,
	DataPromoteOpTypeId = 180,
	MatrixMultiplyOpTypeId = 181,
	PointBoundsOpTypeId = 182,
	RandomRotationOpTypeId = 183,
	V2iVectorDataTypeId = 184,
	V3iVectorDataTypeId = 185,
	ClippingPlaneTypeId = 186,
	ParticleMeshOpTypeId = 187, // obsolete - available for reuse
	HalfDataTypeId = 188,
	MeshPrimitiveOpTypeId = 189,
	PrimitiveEvaluatorTypeId = 190,
	MeshPrimitiveEvaluatorTypeId = 191,
	MeshPrimitiveImplicitSurfaceOpTypeId = 192, // obsolete - available for reuse
	TriangulateOpTypeId = 193,
	SpherePrimitiveEvaluatorTypeId = 194,
	SpherePrimitiveTypeId = 195,
	ConverterTypeId = 196,
	ToCoreConverterTypeId = 197,
	ImageCropOpTypeId = 198,
	MeshPrimitiveShrinkWrapOpTypeId = 199,
	ImagePrimitiveEvaluatorTypeId = 200,
	FromCoreConverterTypeId = 201,
	ShortDataTypeId = 202,
	UShortDataTypeId = 203,
	ShortVectorDataTypeId = 204,
	UShortVectorDataTypeId = 205,
	PathVectorParameterTypeId = 206,
	ColorTransformOpTypeId = 207, // obsolete - available for reuse
	TransformOpTypeId = 208,
	ImageDiffOpTypeId = 209,
	CurvesPrimitiveTypeId = 210,
	CoordinateSystemTypeId = 211,
	MeshNormalsOpTypeId = 212,
	MeshMergeOpTypeId = 213,
	FontTypeId = 214,
	UniformRandomPointDistributionOpTypeId = 215,
	Int64DataTypeId = 216,
	UInt64DataTypeId = 217,
	Int64VectorDataTypeId = 218,
	UInt64VectorDataTypeId = 219,
	MappedRandomPointDistributionOpTypeId = 220,
	PointRepulsionOpTypeId = 221,
	LuminanceOpTypeId = 222,
	ImagePrimitiveOpTypeId = 223,
	ChannelOpTypeId = 224,
	SummedAreaOpTypeId = 225,
	GradeTypeId = 226, // obsolete - available for reuse
	Box2iVectorDataTypeId = 227,
	Box3iVectorDataTypeId = 228,
	MedianCutSamplerTypeId = 229,
	EnvMapSamplerTypeId = 230,
	MeshVertexReorderOpTypeId = 231,
	SplineffDataTypeId = 232,
	SplineddDataTypeId = 233,
	SplinefColor3fDataTypeId = 234,
	SplinefColor4fDataTypeId = 235,
	SplineffParameterTypeId = 236,
	SplineddParameterTypeId = 237,
	SplinefColor3fParameterTypeId = 238,
	SplinefColor4fParameterTypeId = 239,
	CompoundObjectParameterTypeId = 240,
	DisplayDriverTypeId = 241,
	DisplayDriverCreatorTypeId = 242,
	ImageDisplayDriverTypeId = 243,
	DisplayDriverServerTypeId = 244,
	ClientDisplayDriverTypeId = 245,
	SplineToImageTypeId = 246,
	DisplayTypeId = 247,
	MeshTangentsOpTypeId = 248,
	WarpOpTypeId = 249,
	UVDistortOpTypeId = 250,
	LinearToSRGBOpTypeId = 251, // obsolete - available for reuse
	SRGBToLinearOpTypeId = 252, // obsolete - available for reuse
	LinearToCineonOpTypeId = 253, // obsolete - available for reuse
	CineonToLinearOpTypeId = 254, // obsolete - available for reuse
	CubeColorTransformOpTypeId = 255, // obsolete - available for reuse
	CubeColorLookupfDataTypeId = 256, // obsolete - available for reuse
	CubeColorLookupdDataTypeId = 257, // obsolete - available for reuse
	CubeColorLookupfParameterTypeId = 258, // obsolete - available for reuse
	CubeColorLookupdParameterTypeId = 259, // obsolete - available for reuse
	BoolVectorParameterTypeId = 260,
	LinearToRec709OpTypeId = 261, // obsolete - available for reuse
	Rec709ToLinearOpTypeId = 262, // obsolete - available for reuse
	ObjectVectorTypeId = 263,
	ObjectVectorParameterTypeId = 264,
	YUVImageWriterTypeId = 265,
	ImageCompositeOpTypeId = 266,
	ImagePremultiplyOpTypeId = 267,
	ImageUnpremultiplyOpTypeId = 268,
	DateTimeDataTypeId = 269,
	DateTimeParameterTypeId = 270,
	SGIImageReaderTypeId = 271, // obsolete - available for reuse
	TimeDurationDataTypeId = 272,
	TimeDurationParameterTypeId = 273,
	TimePeriodDataTypeId = 274,
	TimePeriodParameterTypeId = 275,
	PatchMeshPrimitiveTypeId = 276,
	CurvesPrimitiveParameterTypeId = 277,
	CurveExtrudeOpTypeId = 278,
	FrameListTypeId = 279,
	EmptyFrameListTypeId = 280,
	FrameRangeTypeId = 281,
	CompoundFrameListTypeId = 282,
	ReorderedFrameListTypeId = 283,
	BinaryFrameListTypeId = 284,
	ReversedFrameListTypeId = 285,
	ExclusionFrameListTypeId = 286,
	FrameListParameterTypeId = 287,
	FileSequenceTypeId = 288,
	FileSequenceParameterTypeId = 289,
	FileSequenceVectorParameterTypeId = 290,
	ParameterisedProceduralTypeId = 291,
	ColorSpaceTransformOpTypeId = 292, // obsolete - available for reuse
	TGAImageReaderTypeId = 293, // obsolete - available for reuse
	TGAImageWriterTypeId = 294, // obsolete - available for reuse
	BINParticleReaderTypeId = 295, // obsolete - available for reuse
	BINParticleWriterTypeId = 296, // obsolete - available for reuse
	BINMeshReaderTypeId = 297, // obsolete - available for reuse
	BGEOParticleReaderTypeId = 298, // obsolete - available for reuse
	NParticleReaderTypeId = 299,
	IFFImageReaderTypeId = 300, // obsolete - available for reuse
	IFFHairReaderTypeId = 301, // obsolete - available for reuse
	FaceAreaOpTypeId = 302,
	CurvesMergeOpTypeId = 303,
	CurvesPrimitiveOpTypeId = 304,
	CurvesPrimitiveEvaluatorTypeId = 305,
	HdrMergeOpTypeId = 306,
	HitMissTransformTypeId = 307, // obsolete - available for reuse
	CurveTracerTypeId = 308,
	ImageThinnerTypeId = 309,
	CurveLineariserTypeId = 310,
	CompoundDataBaseTypeId = 311,
	ImageConvolveOpTypeId = 312, // obsolete - available for reuse
	ClassParameterTypeId = 313,
	ClassVectorParameterTypeId = 314,
	CurveTangentsOpTypeId = 315,
	MarschnerParameterTypeId = 316, // obsolete - available for reuse
	MarschnerLookupTableOpTypeId = 317, // obsolete - available for reuse
	SmoothSkinningDataTypeId = 318,
	FaceVaryingPromotionOpTypeId = 319,
	MeshDistortionsOpTypeId = 320,
	PointVelocityDisplaceOpTypeId = 321,
	SmoothSkinningDataParameterTypeId = 322,
	CompressSmoothSkinningDataOpTypeId = 323,
	DecompressSmoothSkinningDataOpTypeId = 324,
	NormalizeSmoothSkinningWeightsOpTypeId = 325,
	ReorderSmoothSkinningInfluencesOpTypeId = 326,
	RemoveSmoothSkinningInfluencesOpTypeId = 327,
	SmoothSmoothSkinningWeightsOpTypeId = 328,
	MixSmoothSkinningWeightsOpTypeId = 329,
	PointSmoothSkinningOpTypeId = 330,
	AddSmoothSkinningInfluencesOpTypeId = 331,
	LimitSmoothSkinningInfluencesOpTypeId = 332,
	PointsPrimitiveEvaluatorTypeId = 333,
	TransformationMatrixfParameterTypeId = 334,
	TransformationMatrixdParameterTypeId = 335,
	PointsMotionOpTypeId = 336,
	CapturingRendererTypeId = 337,
	LinearToPanalogOpTypeId = 338, // obsolete - available for reuse
	PanalogToLinearOpTypeId = 339, // obsolete - available for reuse
	EnvMapSHProjectorTypeId = 340, // obsolete - available for reuse
	LightTypeId = 341,
	ContrastSmoothSkinningWeightsOpTypeId = 342,
	PointDistributionOpTypeId = 343,
	LineSegment3fDataTypeId = 344,
	LineSegment3dDataTypeId = 345,
	LineSegment3fParameterTypeId = 346,
	LineSegment3dParameterTypeId = 347,
	DataInterleaveOpTypeId = 348,
	DataConvertOpTypeId = 349,
	PNGImageReaderTypeId = 350, // obsolete - available for reuse
	DeepImageReaderTypeId = 351, // obsolete - available for reuse
	DeepImageWriterTypeId = 352, // obsolete - available for reuse
	DeepImageConverterTypeId = 353, // obsolete - available for reuse
	V2iVectorParameterTypeId = 354,
	V3iVectorParameterTypeId = 355,
	DiskPrimitiveTypeId = 356,
	LinearToAlexaLogcOpTypeId = 357, // obsolete - available for reuse
	AlexaLogcToLinearOpTypeId = 358, // obsolete - available for reuse
	ClampOpTypeId = 359,
	MeshFaceFilterOpTypeId = 360,
	TimeCodeDataTypeId = 361,
	TimeCodeParameterTypeId = 362,
	OptionsTypeId = 363,
	MPlayDisplayDriverTypeId = 364,
	SceneInterfaceTypeId = 365,
	SampledSceneInterfaceTypeId = 366,
	SceneCacheTypeId = 367,
	IndexedIOTypeId = 368,
	StreamIndexedIOTypeId = 369,
	FileIndexedIOTypeId = 370,
	MemoryIndexedIOTypeId = 371,
	InternedStringVectorDataTypeId = 372,
	InternedStringDataTypeId = 373,
	LinkedSceneTypeId = 374,
	V2fDataBaseTypeId = 375,
	V2dDataBaseTypeId = 376,
	V2iDataBaseTypeId = 377,
	V3fDataBaseTypeId = 378,
	V3dDataBaseTypeId = 379,
	V3iDataBaseTypeId = 380,
	V2fVectorDataBaseTypeId = 381,
	V2dVectorDataBaseTypeId = 382,
	V2iVectorDataBaseTypeId = 383,
	V3fVectorDataBaseTypeId = 384,
	V3dVectorDataBaseTypeId = 385,
	V3iVectorDataBaseTypeId = 386,
	LensModelTypeId = 387,
	StandardRadialLensModelTypeId = 388,
	LensDistortOpTypeId = 389,
	TransferSmoothSkinningWeightsOpTypeId = 390,
	EXRDeepImageReaderTypeId = 391, // obsolete - available for reuse
	EXRDeepImageWriterTypeId = 392, // obsolete - available for reuse
	ExternalProceduralTypeId = 393,

	// Remember to update TypeIdBinding.cpp !!!

	// If we ever get this far then the core library is too big.
	LastCoreTypeId = 99999,
	// All RunTimeTyped derived classes in extension
	// libraries should use a TypeId in the following range.
	// Don't put the TypeId in here. For python derived classes use
	// the registerTypeId function in RunTimeTypedUtil.py to register the
	// TypeId into the python TypeId enum and check for conflicts.
	FirstExtensionTypeId = 100000,

	FirstCoreDynamicsTypeId = 104000,
	LastCoreDynamicsTypeId = 104999,

	FirstCoreGLTypeId = 105000,
	LastCoreGLTypeId = 105999,

	FirstCoreRITypeId = 106000,
	LastCoreRITypeId = 106999,

	FirstCoreNukeTypeId = 107000,
	LastCoreNukeTypeId = 107999,

	FirstCoreTruelightTypeId = 108000, // Available for reuse
	LastCoreTruelightTypeId = 108999, // Available for reuse

	FirstCoreMayaTypeId = 109000,
	LastCoreMayaTypeId = 109999,

	FirstGafferTypeId = 110000,
	LastGafferTypeId = 110999,
	
	FirstCoreHoudiniTypeId = 111000,
	LastCoreHoudiniTypeId = 111999,

	FirstCoreAlembicTypeId = 112000,
	LastCoreAlembicTypeId = 112999,

	FirstCoreMantraTypeId = 113000,
	LastCoreMantraTypeId = 113999,
	
	FirstCoreArnoldTypeId = 114000,
	LastCoreArnoldTypeId = 114999,

	FirstCoreAppleseedTypeId = 115000,
	LastCoreAppleseedTypeId = 115999,

	// TypeIds dynamically allocated by registerRunTimeTyped (IECore Python)
	FirstDynamicTypeId = 300000,
	LastDynamicTypeId = 399999,

	LastExtensionTypeId = 399999,
	// Any TypeIds beyond this point can be considered safe for private internal use.

};

} // namespace IECore

#endif // IE_CORE_TYPEIDS_H
