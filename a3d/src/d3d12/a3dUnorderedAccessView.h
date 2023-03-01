﻿//-------------------------------------------------------------------------------------------------
// File : a3dUnorderedAccessView.h
// Desc : Unordered Access View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// StorageView class
///////////////////////////////////////////////////////////////////////////////////////////////////
class UnorderedAccessView : public IUnorderedAccessView, public BaseAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    friend struct IUnorderedAccessView;

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
    //! @param[in]      pDevice         デバイス.
    //! @param[in]      pResource       リソースです.
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     pStorageView    ストレージビューの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                        pDevice,
        IResource*                      pResource,
        const UnorderedAccessViewDesc*  pDesc,
        IUnorderedAccessView**          ppStorageView);

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウンタを増やします.
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
    //! @brief      デバッグ名を設定します.
    //! 
    //! @param[in]      name        設定するデバッグ名.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetName(const char* name) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバッグ名を取得します.
    //! 
    //! @return     デバッグ名を返却します.
    //---------------------------------------------------------------------------------------------
    const char* A3D_APIENTRY GetName() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスを取得します.
    //!
    //! @param[out]     ppDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetDevice(IDevice** ppDevice) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタを取得します.
    //!
    //! @return     ディスクリプタを返却します.
    //---------------------------------------------------------------------------------------------
    const Descriptor* A3D_APIENTRY GetDescriptor() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;     //!< 参照カウントです.
    Device*                 m_pDevice;      //!< デバイスです.
    UnorderedAccessViewDesc m_Desc;         //!< 構成設定です.
    IResource*              m_pResource;    //!< リソースです.
    Descriptor*             m_pDescriptor;  //!< ディスクリプタです.
    ObjectName              m_Name;         //!< デバッグ名です.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY UnorderedAccessView();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~UnorderedAccessView();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pTexture    テクスチャです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, IResource* pResource, const UnorderedAccessViewDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    UnorderedAccessView (const UnorderedAccessView&) = delete;
    void operator =     (const UnorderedAccessView&) = delete;
};

} // namespace a3d
