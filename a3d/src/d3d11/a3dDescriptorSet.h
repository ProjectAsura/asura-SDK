﻿//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSet.h
// Desc : DescriptorSet Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSet class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API DescriptorSet : IDescriptorSet, BaseAllocator
{
    //=============================================================================================
    // list of friend classes and mehtods.
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
    //! @param[in]      pDevice             デバイスです.
    //! @param[in]      pDesc               ディスクリプタセットレイアウトです.
    //! @param[in]      ppDescriptors       ディスクリプタの配列です.
    //! @param[out]     ppDescriptorSet     ディスクリプタセットの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                    pDevice,
        DescriptorSetLayoutDesc*    pDesc,
        IDescriptorSet**            ppDescriptorSet);

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
    //! @brief      テクスチャを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetTexture(uint32_t index, ITextureView* pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetBuffer(uint32_t index, IBufferView* pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラーを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pSampler    設定するサンプラーです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetSampler(uint32_t index, ISampler* pSampler) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットを更新します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Update() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ディスクリプタセットを関連付けます.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Bind(ID3D11DeviceContext* pDeviceContext);

    //---------------------------------------------------------------------------------------------
    //! @brief      サブリソースを更新します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY UpdateSubreosurce(ID3D11DeviceContext* pDeviceContext);

private:
    //---------------------------------------------------------------------------------------------
    // Type Definitions.
    //---------------------------------------------------------------------------------------------
    typedef void (*Binder) (ID3D11DeviceContext* pContext, uint32_t slot, void* pResource);
    typedef void (*Updater)(ID3D11DeviceContext* pContext, void* pSubresource);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // BindInfo structure
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct BindInfo
    {
        uint32_t    StageCount;     //!< シェーダステージ数です.
        uint32_t    Slot;           //!< スロット番号です.
        void*       pResource;      //!< リソースです.
        void*       pSubresource;   //!< サブリソースです.
        Binder      Binder[6];      //!< バインダーです.
        Updater     Updater;        //!< アップデータです.
    };

    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;                 //!< 参照カウントです.
    IDevice*                    m_pDevice;                  //!< デバイスです.
    DescriptorSetLayoutDesc*    m_pLayoutDesc;              //!< レイアウト設定です.
    bool                        m_IsGraphicsPipeline;       //!< グラフィックスパイプラインかどうか?
    BindInfo*                   m_pBindInfo;                //!< バインド情報です.

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY DescriptorSet();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~DescriptorSet();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @parma[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(
        IDevice*                    pDevice,
        DescriptorSetLayoutDesc*    pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    DescriptorSet   (const DescriptorSet&) = delete;
    void operator = (const DescriptorSet&) = delete;
};

} // namespace a3d
