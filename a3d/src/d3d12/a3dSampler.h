﻿//-------------------------------------------------------------------------------------------------
// File : a3dSampler.h
// Desc : Sampler Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API Sampler : public ISampler, public BaseAllocator
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
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @param[out]     ppSampler   サンプラーの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(IDevice* pDevice, const SamplerDesc* pDesc, ISampler** ppSampler);

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
    //! @brief      構成設定を取得します.
    //!
    //! @return     構成設定を返却します.
    //---------------------------------------------------------------------------------------------
    D3D12_SAMPLER_DESC A3D_APIENTRY GetD3D12SamplerDesc() const;

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
    D3D12_SAMPLER_DESC      m_Desc;         //!< 構成設定です.
    Descriptor*             m_pDescriptor;  //!< ディスクリプタです.
    ObjectName              m_Name;         //!< デバッグ名です.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY Sampler();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~Sampler();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @parma[in]      pDesc           構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, const SamplerDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    Sampler         (const Sampler&) = delete;
    void operator = (const Sampler&) = delete;
};

} // namespace a3d
