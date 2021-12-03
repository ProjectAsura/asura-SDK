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
class A3D_API DescriptorSet : public IDescriptorSet, public BaseAllocator
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
    //! @brief      定数バッファを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetView(uint32_t index, IConstantBufferView* const pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //! @param[in]      state       リソースステートです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetView(uint32_t index, IShaderResourceView* const pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pResource   設定するリソースです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetView(uint32_t index, IUnorderedAccessView* const pResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラーを設定します.
    //!
    //! @param[in]      index       レイアウト番号です.
    //! @param[in]      pSampler    設定するサンプラーです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY SetSampler(uint32_t index, ISampler* const pSampler) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      コマンドを生成します.
    //!
    //! @param[in]      pCmd        コマンドです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY MakeCommand(ImCmdSetDescriptorSet* pCmd);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>       m_RefCount;                 //!< 参照カウントです.
    Device*                     m_pDevice;                  //!< デバイスです.
    DescriptorSetLayoutDesc*    m_pLayoutDesc;              //!< レイアウト設定です.
    void**                      m_pDescriptors;             //!< ディスクリプタです.
    bool                        m_IsGraphicsPipeline;       //!< グラフィックスパイプラインかどうか?

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
