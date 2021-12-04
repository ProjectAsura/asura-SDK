﻿//-------------------------------------------------------------------------------------------------
// File : a3dTexture.h
// Desc : Texture Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API Texture : public ITexture, public BaseAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOHTING */

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
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppResource      リソースの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*            pDevice,
        const TextureDesc*  pDesc,
        ITexture**          ppResource);

    //---------------------------------------------------------------------------------------------
    //! @brief      ネイティブリソースから生成を行います.
    //!
    //! @param[in]      pDevice             デバイスです.
    //! @param[in]      pNativeResource     ネイティブリソースです.
    //! @param[in]      usage               リソースの使用用途です.
    //! @param[out]     ppResource          リソースの格納先です.
    //! @retval true    生成に成功.
    //! @retval fasle   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY CreateFromNative(
        IDevice*            pDevice,
        ID3D11Texture2D*    pNativeResource,
        RESOURCE_USAGE      usage,
        ITexture**          ppResource);

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
    TextureDesc A3D_APIENTRY GetDesc() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います.
    //!
    //! @return     マッピングしたメモリです.
    //---------------------------------------------------------------------------------------------
    void* A3D_APIENTRY Map() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリマッピングを解除します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Unmap() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      サブリソースレイアウトを取得します.
    //!
    //! @param[in]      subresource     サブリソース番号です.
    //! @return     サブリソースレイアウトを返却します.
    //---------------------------------------------------------------------------------------------
    SubresourceLayout A3D_APIENTRY GetSubresourceLayout(uint32_t subresource) const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソース種別を取得します.
    //!
    //! @return     リソース種別を返却します.
    //---------------------------------------------------------------------------------------------
    RESOURCE_KIND A3D_APIENTRY GetKind() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      1次元テクスチャとして取得します.
    //!
    //! @return     1次元テクスチャを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11Texture1D* A3D_APIENTRY GetAsD3D11Texture1D() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      2次元テクスチャとして取得します.
    //!
    //! @return     2次元テクスチャを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11Texture2D* A3D_APIENTRY GetAsD3D11Texture2D() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      3次元テクスチャとして取得します.
    //!
    //! @return     3次元テクスチャを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11Texture3D* A3D_APIENTRY GetAsD3D11Texture3D() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //!
    //! @return     リソースを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D11Resource* A3D_APIENTRY GetD3D11Resource() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;     //!< 参照カウンタです.
    Device*                 m_pDevice;      //!< デバイスです.
    TextureDesc             m_Desc;         //!< 構成設定です.
    ID3D11Resource*         m_pResource;    //!< リソースです.
    D3D11_MAP               m_MapType;      //!< マップタイプです.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY Texture();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~Texture();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, const TextureDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    Texture         (const Texture&) = delete;
    void operator = (const Texture&) = delete;
};

} // namespace a3d
