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
    /* NONTHING */

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
    //! @brief      シェーダリソースビューを取得します.
    //!
    //! @return     シェーダリソースビューを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11ShaderResourceView* A3D_APIENTRY GetD3D11ShaderResourceView() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューを取得します.
    //!
    //! @return     レンダーターゲットビューを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11RenderTargetView* A3D_APIENTRY GetD3D11RenderTargetView() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      深度ステンシルビューを取得します.
    //!
    //! @return     深度ステンシルビューを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11DepthStencilView* A3D_APIENTRY GetD3D11DepthStencilView() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //!
    //! @return     リソースを返却します.
    //---------------------------------------------------------------------------------------------
    ITexture* A3D_APIENTRY GetResource() const override;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;     //!< 参照カウンタです.
    Device*                     m_pDevice;      //!< デバイスです.
    TextureViewDesc             m_Desc;         //!< 構成設定です.
    Texture*                    m_pTexture;     //!< テクスチャです.
    ID3D11ShaderResourceView*   m_pSRV;         //!< シェーダリソースビューです.
    ID3D11RenderTargetView*     m_pRTV;         //!< レンダーターゲットビューです.
    ID3D11DepthStencilView*     m_pDSV;         //!< 深度ステンシルビューです.

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
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pTexture    テクスチャです.
    //! @param[in]      pDesc       構成設定です.
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

