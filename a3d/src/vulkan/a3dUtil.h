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
//! @param[in]      value       A3D形式です.
//! @return     ネイティブ形式に変換したフォーマットを返却します.
//-------------------------------------------------------------------------------------------------
VkFormat ToNativeFormat(RESOURCE_FORMAT value);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     ネイティブ形式に変換したフォーマットを返却します.
//-------------------------------------------------------------------------------------------------
VkFormat ToNativeViewFormat(RESOURCE_FORMAT value);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     ネイティブ形式に変換した比較オペレータを返却します.
//-------------------------------------------------------------------------------------------------
VkCompareOp ToNativeCompareOp(COMPARE_OP value);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     ネイティブ形式に変換したイメージレイアウトを返却します.
//-------------------------------------------------------------------------------------------------
VkImageLayout ToNativeImageLayout(RESOURCE_STATE value);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      value       A3D形式です.
//! @return     ネイティブ形式に変換したディスクリプタタイプを返却します.
//-------------------------------------------------------------------------------------------------
VkDescriptorType ToNativeDescriptorType(DESCRIPTOR_TYPE value);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]       sampleCount     サンプル数です.
//! @return     ネイティブ形式に変換したサンプル数を返却します.
//-------------------------------------------------------------------------------------------------
VkSampleCountFlagBits ToNativeSampleCountFlags(uint32_t sampleCount);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブ形式に変換します.
//!
//! @param[in]      prop            CPUページプロパティです.
//! @param[in]      isMappable      マップ可能にするかどうか.
//! @return     ネイティブ形式に変換したメモリプロパティフラグを返却します.
//-------------------------------------------------------------------------------------------------
VkMemoryPropertyFlags ToNativeMemoryPropertyFlags(CPU_PAGE_PROPERTY prop, bool isMappable);

//-------------------------------------------------------------------------------------------------
//! @brief      メモリタイプインデックスを取得します.
//!
//! @param[in]      deviceMemoryProps       デバイスメモリプロパティです.
//! @param[in]      requirements            メモリ要件です.
//! @param[in]      flags                   メモリプロパティフラグです.
//! @param[out]     result                  メモリタイプインデックスの格納先です.
//! @retval true    メモリタイプインデックスの取得に成功.
//! @retval fasle   メモリタイプインデックスの取得に失敗.
//-------------------------------------------------------------------------------------------------
bool GetMemoryTypeIndex(
    const VkPhysicalDeviceMemoryProperties& deviceMemoryProps,
    const VkMemoryRequirements&             requirements,
    VkMemoryPropertyFlags                   flags,
    uint32_t&                               result
);

//-------------------------------------------------------------------------------------------------
//! @brief      イメージレイアウトを設定します.
//-------------------------------------------------------------------------------------------------
void SetImageLayout(
    VkCommandBuffer         commandBuffer,
    VkImage                 image,
    VkPipelineStageFlags    srcStageMask,
    VkPipelineStageFlags    dstStageMask,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout,
    VkImageSubresourceRange range
);

//-------------------------------------------------------------------------------------------------
//! @brief      イメージレイアウトを設定します.
//-------------------------------------------------------------------------------------------------
void SetImageLayout(
    VkCommandBuffer         commandBuffer,
    VkPipelineStageFlags    pipelineStage,
    VkImage                 image,
    VkImageLayout           oldLayout,
    VkImageLayout           newLayout,
    VkImageSubresourceRange range
);

//-------------------------------------------------------------------------------------------------
//! @brief      ネイティブアクセスフラグに変換します.
//-------------------------------------------------------------------------------------------------
VkAccessFlags ToNativeAccessFlags(RESOURCE_STATE state);

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソース番号を計算します.
//!
//! @param[in]      mipSlice        ミップ番号.
//! @param[in]      arraySlice      配列番号.
//! @param[in]      planeSlice      平面スライス
//! @param[in]      mipLevels       ミップレベル数.
//! @param[in]      arraySize       配列サイズ.
//! @return     サブリソース番号を返却します.
//-------------------------------------------------------------------------------------------------
uint32_t CalcSubresource(
    uint32_t mipSlice,
    uint32_t arraySlice,
    uint32_t mipLevels,
    uint32_t arraySize);

//-------------------------------------------------------------------------------------------------
//! @brief      サブリソース番号からミップ番号，配列番号を求めます.
//!
//! @param[in]      subresouce      サブリソース番号です.
//! @param[in]      mipLevels       ミップレベル数です.
//! @param[in]      arraySize       配列数です.
//! @param[out]     mipSlice        ミップ番号です.
//! @param[out]     arraySlice      配列番号です
//! @param[out]     planeSlice      平面スライスです.
//-------------------------------------------------------------------------------------------------
void DecomposeSubresource(
    uint32_t subresource,
    uint32_t mipLevels,
    uint32_t arraySize,
    uint32_t& mipSlice,
    uint32_t& arraySlice);


} // namespace a3d