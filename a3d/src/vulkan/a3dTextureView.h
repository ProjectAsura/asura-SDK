﻿//-------------------------------------------------------------------------------------------------
// File : a3dTextureView.h
// Desc : Texture View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextureView class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API TextureView : public ITextureView, public BaseAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      生成処理を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      pTexture        テクスチャです.
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppTextureView   テクスチャビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                pDevice,
        ITexture*               pTexture,
        const TextureViewDesc*  pDesc,
        ITextureView**          ppTextureView);

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY AddRef() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      解放処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Release() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //!
    //! @return     参照カウントを返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetCount() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @param[out]     ppDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetDevice(IDevice** ppDevice) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    TextureViewDesc A3D_APIENTRY GetDesc() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      テクスチャの構成設定を取得します.
    //!
    //! @return     テクスチャの構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    TextureDesc A3D_APIENTRY GetTextureDesc() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      イメージを取得します.
    //!
    //! @return     イメージを返却します.
    //---------------------------------------------------------------------------------------------
    VkImage A3D_APIENTRY GetVulkanImage() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      イメージビューを取得します.
    //!
    //! @return     イメージビューを返却します.
    //---------------------------------------------------------------------------------------------
    VkImageView A3D_APIENTRY GetVulkanImageView() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      イメージアスペクトフラグを取得します.
    //!
    //! @return     イメージアスペクトフラグを返却します.
    //---------------------------------------------------------------------------------------------
    VkImageAspectFlags A3D_APIENTRY GetVulkanImageAspectFlags() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      ステートを取得します.
    //!
    //! @return     ステートを返却します.
    //---------------------------------------------------------------------------------------------
    RESOURCE_STATE A3D_APIENTRY GetState() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;             //!< 参照カウンタです.
    Device*                 m_pDevice;              //!< デバイスです.
    TextureViewDesc         m_Desc;                 //!< 構成設定です.
    Texture*                m_pTexture;             //!< テクスチャです.
    VkImageView             m_ImageView;            //!< イメージビューです.
    VkImageAspectFlags      m_ImageAspectFlags;     //!< アスペクトフラグです.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY TextureView();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~TextureView();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      pTexture        テクスチャです.
    //! @param[in]      pDesc           構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, ITexture* pTexture, const TextureViewDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    TextureView     (const TextureView&) = delete;
    void operator = (const TextureView&) = delete;
};

} // namespace a3d
