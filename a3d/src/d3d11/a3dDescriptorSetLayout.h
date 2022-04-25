﻿//-------------------------------------------------------------------------------------------------
// File : a3dDescriptorSetLayout.h
// Desc : Descriptor Set Layout Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayout class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API DescriptorSetLayout : public IDescriptorSetLayout, public BaseAllocator
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
    //! @param[in]      pDevice         デバイスです。
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppLayout        ディスクリプタセットレイアウトの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                        pDevice,
        const DescriptorSetLayoutDesc*  pDesc,
        IDescriptorSetLayout**          ppLayout);

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

    ////---------------------------------------------------------------------------------------------
    ////! @brief      ディスクリプタセットを割り当てます.
    ////!
    ////! @parma[out]     ppDescriptorSet     ディスクリプタセットの格納先です.
    ////! @retval true    割り当てに成功.
    ////! @retval false   割り当てに失敗.
    ////---------------------------------------------------------------------------------------------
    //bool A3D_APIENTRY CreateDescriptorSet(IDescriptorSet** ppDescriptorSet) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //! 
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    const DescriptorSetLayoutDesc* GetDesc() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;             //!< 参照カウントです.
    Device*                 m_pDevice;              //!< デバイスです.
    DescriptorSetLayoutDesc m_Desc;                 //!< 構成設定です.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY DescriptorSetLayout();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~DescriptorSetLayout();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, const DescriptorSetLayoutDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    DescriptorSetLayout (const DescriptorSetLayout&) = delete;
    void operator =     (const DescriptorSetLayout&) = delete;
};

} // namespace a3d
