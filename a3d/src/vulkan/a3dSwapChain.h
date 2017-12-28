﻿//-------------------------------------------------------------------------------------------------
// File : a3dSwapChain.h
// Desc : SwapChain Implementation.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

//-------------------------------------------------------------------------------------------------
// Forward Declarations.
//-------------------------------------------------------------------------------------------------
class Texture;


///////////////////////////////////////////////////////////////////////////////////////////////////
// SwapChain class
///////////////////////////////////////////////////////////////////////////////////////////////////
class A3D_API SwapChain : public ISwapChain, public BaseAllocator
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
    //! @param[in]      pDesc           構成設定です.
    //! @param[out]     ppSwapChain     スワップチェインの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //---------------------------------------------------------------------------------------------
    static bool A3D_APIENTRY Create(
        IDevice*                pDevice, 
        const SwapChainDesc*    pDesc,
        ISwapChain**            ppSwapChain);

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
    SwapChainDesc A3D_APIENTRY GetDesc() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      画面に表示します.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Present() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      現在のバッファ番号を取得します.
    //!
    //! @return     現在のバッファ番号を返却します.
    //---------------------------------------------------------------------------------------------
    uint32_t A3D_APIENTRY GetCurrentBufferIndex() override;

    //---------------------------------------------------------------------------------------------
    //! @brief      指定されたバッファを取得します.
    //!
    //! @param[in]      index       バッファ番号です.
    //! @param[out]     ppResource  リソースの格納先です.
    //! @retval true    バッファの取得に成功.
    //! @retval false   バッファの取得に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY GetBuffer(uint32_t index, ITexture** ppResource) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      バッファをリサイズします.
    //!
    //! @param[in]      width       リサイズする横幅.
    //! @param[in]      height      リサイズする縦幅.
    //! @retval true    リサイズに成功.
    //! @retval false   リサイズに失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY ResizeBuffers(uint32_t width, uint32_t height) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      メタデータを設定します.
    //!
    //! @param[in]      type        メタデータタイプです.
    //! @param[in]      pData       メタデータです.
    //! @retval true    メタデータの設定に成功.
    //! @retval false   メタデータの設定に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY SetMetaData(META_DATA_TYPE type, void* pData) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      色空間がサポートされているかチェックします.
    //!
    //! @param[in]      type        色空間タイプです.
    //! @retval true    チェックに成功.
    //! @retval false   チェックに失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY CheckColorSpaceSupport(COLOR_SPACE_TYPE type) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      色空間を設定します.
    //!
    //! @param[in]      type        色空間タイプです.
    //! @retval true    設定に成功.
    //! @retval false   設定に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY SetColorSpace(COLOR_SPACE_TYPE type) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フルスクリーンモードかどうかチェックします.
    //!
    //! @retval true    フルスクリーンモードです.
    //! @retval false   ウィンドウモードです.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY IsFullScreenMode() const override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フルスクリーンモードを設定します.
    //!
    //! @param[in]      enable      フルスクリーンにする場合は true を，ウィンドウモードにする場合は falseを指定します.
    //! @retval true    設定に成功.
    //! @retval false   設定に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY SetFullScreenMode(bool enable) override;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic<uint32_t>           m_RefCount;             //!< 参照カウンタです.
    Device*                         m_pDevice;              //!< デバイスです.
    Queue*                          m_pQueue;               //!< コマンドキューです.
    SwapChainDesc                   m_Desc;                 //!< 構成設定です.
    VkSurfaceKHR                    m_Surface;              //!< サーフェイスです.
    VkSwapchainKHR                  m_SwapChain;            //!< スワップチェインです.
    Texture**                       m_pBuffers;             //!< バッファです.
    VkImage*                        m_pImages;              //!< イメージです.
    VkImageView*                    m_pImageViews;          //!< イメージビューです.
    uint32_t                        m_CurrentBufferIndex;   //!< 現在のバッファ番号です.
    VkPresentModeKHR                m_PresentMode;          //!< 表示モード.
    VkFormat                        m_ImageFormat;          //!< イメージフォーマット.
    VkColorSpaceKHR                 m_ColorSpace;           //!< カラースペース.
    bool                            m_IsFullScreen;         //!< フルスクリーンかどうか?
    VkSurfaceTransformFlagBitsKHR   m_PreTransform;         //!< サーフェイス変換フラグ.
    uint32_t                        m_SurfaceFormatCount;   //!< サーフェイスフォーマット数.
    VkSurfaceFormatKHR*             m_pSurfaceFormats;      //!< サーフェイスフォーマット.

    #if A3D_IS_WIN
        HWND                m_hWnd;
        HINSTANCE           m_hInstance;
        RECT                m_Rect;
        LONG                m_WindowStyle;
    #elif A3D_IS_LINUX
        // NOTHING
    #elif A3D_IS_ANDROID
        // NOTHING
    #endif

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    A3D_APIENTRY SwapChain();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //--------------------------------------------------------------------------------------------
    A3D_APIENTRY ~SwapChain();

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY Init(IDevice* pDevice, const SwapChainDesc* pDesc);

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void A3D_APIENTRY Term();

    //---------------------------------------------------------------------------------------------
    //! @brief      サーフェイスの初期化を行います.
    //!
    //! @param[in]      pSurface        初期化するサーフェイス.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //---------------------------------------------------------------------------------------------
    bool A3D_APIENTRY InitSurface(VkSurfaceKHR* pSurface);

    SwapChain       (const SwapChain&) = delete;
    void operator = (const SwapChain&) = delete;
};

} // namespace a3d
