﻿//-------------------------------------------------------------------------------------------------
// File : a3dAccelerationStructure.h
// Desc : Acceleration Structure For Ray Tracing.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// AccelerationStructure class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API AccelerationStructure : public IAccelerationStructure, public BaseAllocator
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
    //! @param[out]     ppAS        加速機能の格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗. 
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                            pDevice,
        const AccelerationStructureDesc*    pDesc,
        IAccelerationStructure**            ppAS);

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
    //! @brief      デバイスを取得します.
    //! 
    //! @param[out]     ppDevice        デバイスの格納先です.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY GetDevice(IDevice** ppDevice) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスアドレスを取得します.
    //! 
    //! @return     デバイスアドレスを返却します.
    //---------------------------------------------------------------------------------------------
    uint64_t A3D_APIENTRY GetDeviceAddress() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ビルドします.
    //! 
    //! @param[in]      pCommandList    コマンドリストです.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Build(ID3D12GraphicsCommandList6* pCommandList);

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //! 
    //! @return     リソースを返却します.
    //---------------------------------------------------------------------------------------------
    ID3D12Resource* A3D_APIENTRY GetD3D12Resource() const;

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ResourceHolder structure
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct ResourceHolder
    {
        ID3D12Resource*         pResource   = nullptr;
        D3D12MA::Allocation*    pAllocation = nullptr;

        void Release()
        {
            SafeRelease(pResource);
            SafeRelease(pAllocation);
        }
    };

    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>                               m_RefCount;
    Device*                                             m_pDevice;
    ResourceHolder                                      m_Scratch;
    ResourceHolder                                      m_Structure;
    D3D12_RAYTRACING_GEOMETRY_DESC*                     m_pGeometryDescs;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC  m_BuildDesc;

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY AccelerationStructure();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~AccelerationStructure();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, const AccelerationStructureDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    AccelerationStructure   (const AccelerationStructure&) = delete;
    void operator =         (const AccelerationStructure&) = delete;
};

} // namespace a3d
