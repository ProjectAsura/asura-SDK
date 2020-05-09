//-----------------------------------------------------------------------------
// File : a3dSpirv.cpp
// Desc : SPIR-V Parser.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "a3dSpirv.h"


namespace {

///////////////////////////////////////////////////////////////////////////////
// SpvHeader structure
///////////////////////////////////////////////////////////////////////////////
struct SpvHeader
{
    uint32_t    Magic;
    uint32_t    Version;
    uint32_t    Generator;
    uint32_t    Bound;
    uint32_t    Schema;
};

//-----------------------------------------------------------------------------
//      返り値とデータ型を持つか判定します.
//-----------------------------------------------------------------------------
inline void HasResultAndType
(
    a3d::SpvOp  opcode,
    bool&       hasResult,
    bool&       hasResultType
)
{
    hasResult = hasResultType = false;
    switch (opcode) 
    {
        default: break;
        case a3d::SpvOpNop                                                                      : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpUndef                                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSourceContinued                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpSource                                                                   : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpSourceExtension                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpName                                                                     : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpMemberName                                                               : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpString                                                                   : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpLine                                                                     : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpExtension                                                                : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpExtInstImport                                                            : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpExtInst                                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpMemoryModel                                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpEntryPoint                                                               : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpExecutionMode                                                            : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpCapability                                                               : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpTypeVoid                                                                 : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeBool                                                                 : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeInt                                                                  : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeFloat                                                                : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeVector                                                               : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeMatrix                                                               : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeImage                                                                : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeSampler                                                              : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeSampledImage                                                         : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeArray                                                                : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeRuntimeArray                                                         : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeStruct                                                               : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeOpaque                                                               : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypePointer                                                              : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeFunction                                                             : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeEvent                                                                : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeDeviceEvent                                                          : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeReserveId                                                            : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeQueue                                                                : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypePipe                                                                 : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeForwardPointer                                                       : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpConstantTrue                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConstantFalse                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConstant                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConstantComposite                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConstantSampler                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConstantNull                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSpecConstantTrue                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSpecConstantFalse                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSpecConstant                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSpecConstantComposite                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSpecConstantOp                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFunction                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFunctionParameter                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFunctionEnd                                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpFunctionCall                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpVariable                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageTexelPointer                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLoad                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpStore                                                                    : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpCopyMemory                                                               : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpCopyMemorySized                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpAccessChain                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpInBoundsAccessChain                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpPtrAccessChain                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpArrayLength                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGenericPtrMemSemantics                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpInBoundsPtrAccessChain                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDecorate                                                                 : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpMemberDecorate                                                           : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpDecorationGroup                                                          : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpGroupDecorate                                                            : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpGroupMemberDecorate                                                      : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpVectorExtractDynamic                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpVectorInsertDynamic                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpVectorShuffle                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCompositeConstruct                                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCompositeExtract                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCompositeInsert                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCopyObject                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpTranspose                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSampledImage                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleImplicitLod                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleExplicitLod                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleDrefImplicitLod                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleDrefExplicitLod                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleProjImplicitLod                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleProjExplicitLod                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleProjDrefImplicitLod                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleProjDrefExplicitLod                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageFetch                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageGather                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageDrefGather                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageRead                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageWrite                                                               : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpImage                                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageQueryFormat                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageQueryOrder                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageQuerySizeLod                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageQuerySize                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageQueryLod                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageQueryLevels                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageQuerySamples                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConvertFToU                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConvertFToS                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConvertSToF                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConvertUToF                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUConvert                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSConvert                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFConvert                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpQuantizeToF16                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConvertPtrToU                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSatConvertSToU                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSatConvertUToS                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpConvertUToPtr                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpPtrCastToGeneric                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGenericCastToPtr                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGenericCastToPtrExplicit                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitcast                                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSNegate                                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFNegate                                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIAdd                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFAdd                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpISub                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFSub                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIMul                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFMul                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUDiv                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSDiv                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFDiv                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUMod                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSRem                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSMod                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFRem                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFMod                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpVectorTimesScalar                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpMatrixTimesScalar                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpVectorTimesMatrix                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpMatrixTimesVector                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpMatrixTimesMatrix                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpOuterProduct                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDot                                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIAddCarry                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpISubBorrow                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUMulExtended                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSMulExtended                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAny                                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAll                                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIsNan                                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIsInf                                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIsFinite                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIsNormal                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSignBitSet                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLessOrGreater                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpOrdered                                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUnordered                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLogicalEqual                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLogicalNotEqual                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLogicalOr                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLogicalAnd                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLogicalNot                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSelect                                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIEqual                                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpINotEqual                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUGreaterThan                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSGreaterThan                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUGreaterThanEqual                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSGreaterThanEqual                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpULessThan                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSLessThan                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpULessThanEqual                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSLessThanEqual                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFOrdEqual                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFUnordEqual                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFOrdNotEqual                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFUnordNotEqual                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFOrdLessThan                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFUnordLessThan                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFOrdGreaterThan                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFUnordGreaterThan                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFOrdLessThanEqual                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFUnordLessThanEqual                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFOrdGreaterThanEqual                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFUnordGreaterThanEqual                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpShiftRightLogical                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpShiftRightArithmetic                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpShiftLeftLogical                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitwiseOr                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitwiseXor                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitwiseAnd                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpNot                                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitFieldInsert                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitFieldSExtract                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitFieldUExtract                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitReverse                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBitCount                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDPdx                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDPdy                                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFwidth                                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDPdxFine                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDPdyFine                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFwidthFine                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDPdxCoarse                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDPdyCoarse                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFwidthCoarse                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpEmitVertex                                                               : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpEndPrimitive                                                             : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpEmitStreamVertex                                                         : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpEndStreamPrimitive                                                       : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpControlBarrier                                                           : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpMemoryBarrier                                                            : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpAtomicLoad                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicStore                                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpAtomicExchange                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicCompareExchange                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicCompareExchangeWeak                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicIIncrement                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicIDecrement                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicIAdd                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicISub                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicSMin                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicUMin                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicSMax                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicUMax                                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicAnd                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicOr                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicXor                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpPhi                                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpLoopMerge                                                                : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpSelectionMerge                                                           : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpLabel                                                                    : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpBranch                                                                   : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpBranchConditional                                                        : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpSwitch                                                                   : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpKill                                                                     : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpReturn                                                                   : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpReturnValue                                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpUnreachable                                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpLifetimeStart                                                            : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpLifetimeStop                                                             : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpGroupAsyncCopy                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupWaitEvents                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpGroupAll                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupAny                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupBroadcast                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupIAdd                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupFAdd                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupFMin                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupUMin                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupSMin                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupFMax                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupUMax                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupSMax                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpReadPipe                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpWritePipe                                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpReservedReadPipe                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpReservedWritePipe                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpReserveReadPipePackets                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpReserveWritePipePackets                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCommitReadPipe                                                           : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpCommitWritePipe                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpIsValidReserveId                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetNumPipePackets                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetMaxPipePackets                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupReserveReadPipePackets                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupReserveWritePipePackets                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupCommitReadPipe                                                      : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpGroupCommitWritePipe                                                     : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpEnqueueMarker                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpEnqueueKernel                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetKernelNDrangeSubGroupCount                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetKernelNDrangeMaxSubGroupSize                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetKernelWorkGroupSize                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetKernelPreferredWorkGroupSizeMultiple                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRetainEvent                                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpReleaseEvent                                                             : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpCreateUserEvent                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIsValidEvent                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSetUserEventStatus                                                       : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpCaptureEventProfilingInfo                                                : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpGetDefaultQueue                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBuildNDRange                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleImplicitLod                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleExplicitLod                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleDrefImplicitLod                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleDrefExplicitLod                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleProjImplicitLod                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleProjExplicitLod                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleProjDrefImplicitLod                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseSampleProjDrefExplicitLod                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseFetch                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseGather                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseDrefGather                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSparseTexelsResident                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpNoLine                                                                   : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpAtomicFlagTestAndSet                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAtomicFlagClear                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpImageSparseRead                                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSizeOf                                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpTypePipeStorage                                                          : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpConstantPipeStorage                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCreatePipeFromPipeStorage                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetKernelLocalSizeForSubgroupCount                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGetKernelMaxNumSubgroups                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpTypeNamedBarrier                                                         : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpNamedBarrierInitialize                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpMemoryNamedBarrier                                                       : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpModuleProcessed                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpExecutionModeId                                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpDecorateId                                                               : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpGroupNonUniformElect                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformAll                                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformAny                                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformAllEqual                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBroadcast                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBroadcastFirst                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBallot                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformInverseBallot                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBallotBitExtract                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBallotBitCount                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBallotFindLSB                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBallotFindMSB                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformShuffle                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformShuffleXor                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformShuffleUp                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformShuffleDown                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformIAdd                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformFAdd                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformIMul                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformFMul                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformSMin                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformUMin                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformFMin                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformSMax                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformUMax                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformFMax                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBitwiseAnd                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBitwiseOr                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformBitwiseXor                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformLogicalAnd                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformLogicalOr                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformLogicalXor                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformQuadBroadcast                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformQuadSwap                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCopyLogical                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpPtrEqual                                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpPtrNotEqual                                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpPtrDiff                                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupBallotKHR                                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupFirstInvocationKHR                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAllKHR                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAnyKHR                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAllEqualKHR                                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupReadInvocationKHR                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpTypeRayQueryProvisionalKHR                                               : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpRayQueryInitializeKHR                                                    : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpRayQueryTerminateKHR                                                     : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpRayQueryGenerateIntersectionKHR                                          : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpRayQueryConfirmIntersectionKHR                                           : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpRayQueryProceedKHR                                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionTypeKHR                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupIAddNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupFAddNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupFMinNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupUMinNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupSMinNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupFMaxNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupUMaxNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupSMaxNonUniformAMD                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFragmentMaskFetchAMD                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpFragmentFetchAMD                                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpReadClockKHR                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpImageSampleFootprintNV                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpGroupNonUniformPartitionNV                                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpWritePackedPrimitiveIndices4x8NV                                         : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpReportIntersectionNV                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIgnoreIntersectionNV                                                     : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpTerminateRayNV                                                           : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpTraceNV                                                                  : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpTypeAccelerationStructureNV                                              : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpExecuteCallableNV                                                        : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpTypeCooperativeMatrixNV                                                  : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpCooperativeMatrixLoadNV                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCooperativeMatrixStoreNV                                                 : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpCooperativeMatrixMulAddNV                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpCooperativeMatrixLengthNV                                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpBeginInvocationInterlockEXT                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpEndInvocationInterlockEXT                                                : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpDemoteToHelperInvocationEXT                                              : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpIsHelperInvocationEXT                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupShuffleINTEL                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupShuffleDownINTEL                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupShuffleUpINTEL                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupShuffleXorINTEL                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupBlockReadINTEL                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupBlockWriteINTEL                                                  : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpSubgroupImageBlockReadINTEL                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupImageBlockWriteINTEL                                             : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpSubgroupImageMediaBlockReadINTEL                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupImageMediaBlockWriteINTEL                                        : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpUCountLeadingZerosINTEL                                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUCountTrailingZerosINTEL                                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAbsISubINTEL                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpAbsUSubINTEL                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIAddSatINTEL                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUAddSatINTEL                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIAverageINTEL                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUAverageINTEL                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIAverageRoundedINTEL                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUAverageRoundedINTEL                                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpISubSatINTEL                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUSubSatINTEL                                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpIMul32x16INTEL                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpUMul32x16INTEL                                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpDecorateString                                                           : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpMemberDecorateString                                                     : { hasResult = false;  hasResultType = false;  } break;
        case a3d::SpvOpVmeImageINTEL                                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpTypeVmeImageINTEL                                                        : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcImePayloadINTEL                                                   : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcRefPayloadINTEL                                                   : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcSicPayloadINTEL                                                   : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcMcePayloadINTEL                                                   : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcMceResultINTEL                                                    : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcImeResultINTEL                                                    : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcImeResultSingleReferenceStreamoutINTEL                            : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcImeResultDualReferenceStreamoutINTEL                              : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcImeSingleReferenceStreaminINTEL                                   : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcImeDualReferenceStreaminINTEL                                     : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcRefResultINTEL                                                    : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpTypeAvcSicResultINTEL                                                    : { hasResult = true;   hasResultType = false;  } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetInterShapePenaltyINTEL                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetInterDirectionPenaltyINTEL                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetMotionVectorCostFunctionINTEL                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetAcOnlyHaarINTEL                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceConvertToImePayloadINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceConvertToImeResultINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceConvertToRefPayloadINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceConvertToRefResultINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceConvertToSicPayloadINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceConvertToSicResultINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetMotionVectorsINTEL                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetInterDistortionsINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetBestInterDistortionsINTEL                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetInterMajorShapeINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetInterMinorShapeINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetInterDirectionsINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetInterMotionVectorCountINTEL                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetInterReferenceIdsINTEL                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeInitializeINTEL                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeSetSingleReferenceINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeSetDualReferenceINTEL                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeRefWindowSizeINTEL                                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeAdjustRefOffsetINTEL                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeConvertToMcePayloadINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeSetMaxMotionVectorCountINTEL                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeSetUnidirectionalMixDisableINTEL                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeSetWeightedSadINTEL                                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithSingleReferenceINTEL                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithDualReferenceINTEL                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeConvertToMceResultINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetSingleReferenceStreaminINTEL                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetDualReferenceStreaminINTEL                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeStripSingleReferenceStreamoutINTEL                         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeStripDualReferenceStreamoutINTEL                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetBorderReachedINTEL                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetTruncatedSearchIndicationINTEL                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcFmeInitializeINTEL                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcBmeInitializeINTEL                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefConvertToMcePayloadINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefSetBidirectionalMixDisableINTEL                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefSetBilinearFilterEnableINTEL                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefEvaluateWithSingleReferenceINTEL                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefEvaluateWithDualReferenceINTEL                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefEvaluateWithMultiReferenceINTEL                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcRefConvertToMceResultINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicInitializeINTEL                                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicConfigureSkcINTEL                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicConfigureIpeLumaINTEL                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicConfigureIpeLumaChromaINTEL                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetMotionVectorMaskINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicConvertToMcePayloadINTEL                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicSetBilinearFilterEnableINTEL                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicSetSkcForwardTransformEnableINTEL                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL                               : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicEvaluateIpeINTEL                                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicEvaluateWithSingleReferenceINTEL                           : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicEvaluateWithDualReferenceINTEL                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicEvaluateWithMultiReferenceINTEL                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicConvertToMceResultINTEL                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetIpeLumaShapeINTEL                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetBestIpeLumaDistortionINTEL                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetBestIpeChromaDistortionINTEL                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetPackedIpeLumaModesINTEL                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetIpeChromaModeINTEL                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL                        : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpSubgroupAvcSicGetInterRawSadsINTEL                                       : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetRayTMinKHR                                                    : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetRayFlagsKHR                                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionTKHR                                              : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionInstanceCustomIndexKHR                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionInstanceIdKHR                                     : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR         : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionGeometryIndexKHR                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionPrimitiveIndexKHR                                 : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionBarycentricsKHR                                   : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionFrontFaceKHR                                      : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionCandidateAABBOpaqueKHR                            : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionObjectRayDirectionKHR                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionObjectRayOriginKHR                                : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetWorldRayDirectionKHR                                          : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetWorldRayOriginKHR                                             : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionObjectToWorldKHR                                  : { hasResult = true;   hasResultType = true;   } break;
        case a3d::SpvOpRayQueryGetIntersectionWorldToObjectKHR                                  : { hasResult = true;   hasResultType = true;   } break;
    }
}

} // namespace


namespace a3d {

//-----------------------------------------------------------------------------
//      エントリーポイント名を探します.
//-----------------------------------------------------------------------------
const char* FindEntryPoint(const void* pBinary, size_t binarySize)
{
    auto ptr = reinterpret_cast<const uint32_t*>(pBinary);
    auto end = ptr + binarySize;

    auto header = reinterpret_cast<const SpvHeader*>(ptr);
    ptr += (sizeof(SpvHeader) / sizeof(uint32_t));

    if (header->Magic != 0x07230203)
    { return nullptr; }

    while(ptr != end)
    {
        auto token  = *ptr;
        auto opcode = uint16_t(token & 0x0000ffff);
        auto count  = uint16_t((token & 0xffff0000) >> 16);

        switch(opcode)
        {
        case a3d::SpvOpEntryPoint:
            {
                ptr++;
                auto var1 = (*ptr++);   // Execuetion Model.
                auto var2 = (*ptr++);   // Entry Point Id.
                auto var3 = ptr;        // Lateral Name.
                return reinterpret_cast<const char*>(var3);
            }
            break;

        default:
             ptr += count;
             break;
        }
    }

    return nullptr;
}


} // namespace a3d