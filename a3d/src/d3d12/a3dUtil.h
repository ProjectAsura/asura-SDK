﻿//-------------------------------------------------------------------------------------------------
// File : a3dUtil.h
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      format      A3D形式です.
//! @return     ネイティブ形式に変換したフォーマットを返却します.
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT ToNativeFormat(RESOURCE_FORMAT format);

//-------------------------------------------------------------------------------------------------
//! @brief      タイプレスのネイティブ形式に変換します.
//!
//! @param[in]      format      A3D形式です.
//! @return     タイプレスのネイティブ形式に変換したフォーマットを返却します. 
//!             タイプレス形式が無い場合は該当するDXGI_FORMATを返却します.
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT ToNativeTypelessFormat(RESOURCE_FORMAT format);

//-------------------------------------------------------------------------------------------------
//! @brief      ビューフォーマットに変換します.
//!
//! @param[in]      format      A3D形式です.
//! @param[in]      isStencil   ステンシルテクスチャとする場合はtrueを指定.
//! @return     ビューフォーマットに適した形式のフォーマットを返却します. 
//-------------------------------------------------------------------------------------------------
DXGI_FORMAT ToNativeViewFormat(RESOURCE_FORMAT format, bool isStencil);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式からA3D形式に変換します.
//!
//! @param[in]      format      DXGI形式です.
//! @return     ネイティブ形式からA3D形式に変換したフォーマットを返却します.
//-------------------------------------------------------------------------------------------------
RESOURCE_FORMAT ToWrapFormat(DXGI_FORMAT format);

//-------------------------------------------------------------------------------------------------
//! @brief      リソースフォーマットのビット数に変換します.
//!
//! @param[in]      format      リソースフォーマットです.
//! @return     リソースフォーマットのビット数を返却します.
//-------------------------------------------------------------------------------------------------
uint32_t ToBits(RESOURCE_FORMAT format);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      state       A3D形式です.
//! @return     ネイティブ形式に変換したステートを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_RESOURCE_STATES ToNativeState(RESOURCE_STATE state);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      dimension       A3D形式です.
//! @return     ネイティブ形式に変換した次元を返却します.
//-------------------------------------------------------------------------------------------------
D3D12_RESOURCE_DIMENSION ToNativeResorceDimension(RESOURCE_DIMENSION dimension);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式からA3D形式に変換します.
//!
//! @param[in]      dimension       D3D12形式です.
//! @return     ネイティブ形式からA3D形式に変換した次元を返却します.
//-------------------------------------------------------------------------------------------------
RESOURCE_DIMENSION ToWrapDimension(D3D12_RESOURCE_DIMENSION dimension);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      dimension       A3D形式です.
//! @return     ネイティブ形式に変換したリソースフラグを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_RESOURCE_FLAGS ToNativeResourceFlags(uint32_t usage);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      dimension       A3D形式です.
//! @return     ネイティブ形式に変換したリソースフラグを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_COMPARISON_FUNC ToNativeComparisonFunc(COMPARE_OP operation);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      topology       A3D形式です.
//! @return     ネイティブ形式に変換したプリミティブトポロジータイプを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_PRIMITIVE_TOPOLOGY_TYPE ToNativePrimitiveTopology(PRIMITIVE_TOPOLOGY topology);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      topology        A3D形式です.
//! @param[in]      controlCount    制御パッチ数です.
//! @return     ネイティブ形式に変換したプリミティブトポロジーを返却します.
//-------------------------------------------------------------------------------------------------
D3D_PRIMITIVE_TOPOLOGY ToNativePrimitive(PRIMITIVE_TOPOLOGY topology, uint32_t controlCount);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      type        A3D形式です.
//! @return     ネイティブ形式に変換したヒープタイプを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_HEAP_TYPE ToNativeHeapType(HEAP_TYPE type);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      prop        A3D形式です.
//! @return     ネイティブ形式に変換したCPUページプロパティを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_CPU_PAGE_PROPERTY ToNativeCpuPageProperty(CPU_PAGE_PROPERTY prop);

//-------------------------------------------------------------------------------------------------
//! @brief      レンダーターゲットビューディメンションに変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     変換したレンダーターゲットビューディメンションを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_RTV_DIMENSION ToNativeRTVDimension(VIEW_DIMENSION value);

//-------------------------------------------------------------------------------------------------
//! @brief      深度ステンシルビューディメンションに変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     変換した深度ステンシルビューディメンションを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_DSV_DIMENSION ToNativeDSVDimension(VIEW_DIMENSION value);

//-------------------------------------------------------------------------------------------------
//! @brief      シェーダリソースビューに変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     変換したシェーダリソースビューディメンションを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_SRV_DIMENSION ToNativeSRVDimension(VIEW_DIMENSION value);

//-------------------------------------------------------------------------------------------------
//! @brief      アンオーダードアクセスビューに変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     変換したアンオーダードアクセスビューディメンションを返却します.
//-------------------------------------------------------------------------------------------------
D3D12_UAV_DIMENSION ToNativeUAVDimension(VIEW_DIMENSION value);

//-------------------------------------------------------------------------------------------------
//! @brief      カラースペースタイプを変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     変換したカラースペースタイプを返却します.
//-------------------------------------------------------------------------------------------------
DXGI_COLOR_SPACE_TYPE ToNativeColorSpace(COLOR_SPACE_TYPE value);

} // namespace a3d
