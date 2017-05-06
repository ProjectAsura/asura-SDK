//-------------------------------------------------------------------------------------------------
// File : a3dBuffer.h
// Desc : Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API Buffer : public IBuffer, public BaseAllocator
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
        const BufferDesc*   pDesc,
        IBuffer**           ppResource);

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
    BufferDesc A3D_APIENTRY GetDesc() const override;

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
    //! @brief      バッファを取得します.
    //!
    //! @return     バッファを返却します.
    //---------------------------------------------------------------------------------------------
    VkBuffer A3D_APIENTRY GetVulkanBuffer() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      デバイスメモリを取得します.
    //!
    //! @return     デバイスメモリを返却します.
    //---------------------------------------------------------------------------------------------
    VkDeviceMemory A3D_APIENTRY GetVulkanDeviceMemory() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリ要件を取得します.
    //!
    //! @return     メモリ要件を返却します.
    //---------------------------------------------------------------------------------------------
    VkMemoryRequirements A3D_APIENTRY GetVulkanMemoryRequirements() const;

    //---------------------------------------------------------------------------------------------
    //! @brief      リソースタイプを取得します.
    //!
    //! @return     リソースタイプを返却します.
    //---------------------------------------------------------------------------------------------
    RESOURCE_KIND A3D_APIENTRY GetKind() const override;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>   m_RefCount;             //!< 参照カウンタです.
    Device*                 m_pDevice;              //!< デバイスです.
    BufferDesc              m_Desc;                 //!< 構成設定です.
    VkBuffer                m_Buffer;               //!< バッファです.
    VkDeviceMemory          m_DeviceMemory;         //!< デバイスメモリです.
    VkMemoryRequirements    m_MemoryRequirements;   //!< メモリ要件です.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY Buffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY ~Buffer();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, const BufferDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    Buffer          (const Buffer&) = delete;
    void operator = (const Buffer&) = delete;
};


} // namespace a3d
