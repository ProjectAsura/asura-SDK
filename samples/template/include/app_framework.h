//-------------------------------------------------------------------------------------------------
// File : app_framework.h
// Desc : Framework
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <a3dx_app.h>
#include <a3dx_math.h>
#include <a3dx_gui.h>
#include <list>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Framework class
///////////////////////////////////////////////////////////////////////////////////////////////////
class Framework 
    : public a3d::IMouseHandler
    , public a3d::IKeyHandler
    , public a3d::IResizeHandler
    , public a3d::ITypingHandler
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
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    Framework();

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~Framework();

    //---------------------------------------------------------------------------------------------
    //! @brief      実行します.
    //!
    //! @param[in]      width           ウィンドウの横幅.
    //! @param[in]      height          ウィンドウの縦幅.
    //! @param[in]      colorFormat     バックバッファフォーマット.
    //! @param[in]      depthFormat     深度バッファフォーマット.
    //---------------------------------------------------------------------------------------------
    void Run(
        uint32_t width,
        uint32_t height,
        a3d::RESOURCE_FORMAT colorFormat,
        a3d::RESOURCE_FORMAT depthFormat);

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    a3d::IDevice*       m_pDevice           = nullptr;
    a3d::ISwapChain*    m_pSwapChain        = nullptr;
    a3d::IQueue*        m_pGraphicsQueue    = nullptr;
    a3d::IFence*        m_pFence            = nullptr;
    a3d::ICommandList*  m_pCommandList[2]   = {};
    a3d::IFrameBuffer*  m_pFrameBuffer[2]   = {};
    a3d::ITexture*      m_pColorBuffer[2]   = {};
    a3d::ITextureView*  m_pColorView[2]     = {};
    a3d::ITexture*      m_pDepthBuffer      = nullptr;
    a3d::ITextureView*  m_pDepthView        = nullptr;

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    a3d::ICommandList* GetCurrentCommandList() const;
    a3d::IFrameBuffer* GetCurrentFrameBuffer() const;
    a3d::ITexture*     GetCurrentColorBuffer() const;
    a3d::ITextureView* GetCurrentColorView  () const;
    a3d::IApp*         GetApp () const;
    bool               IsReady() const;
    void               ToggleFullScreen();
    void               AddToDisposer(a3d::IReference* pItem, uint32_t life = 3);

    virtual void OnMouse (const a3d::MouseEventArg&)    {}
    virtual void OnKey   (const a3d::KeyEventArg&)      {}
    virtual void OnResize(const a3d::ResizeEventArg&)   {}
    virtual void OnTyping(uint32_t keyCode)             {}
    virtual bool OnInit  () { return true; }
    virtual void OnTerm  () {}
    virtual void OnDraw  () {}

private:
    //////////////////////////////////////////////////////////////////////////////////////////////
    // DisposeItem structure
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct DisposeItem
    {
        a3d::IReference*    pItem;
        uint32_t            life;
    };

    //=============================================================================================
    // private variables.
    //=============================================================================================
    a3d::IApp*              m_pApp          = nullptr;
    bool                    m_Ready         = false;
    bool                    m_FullScreen    = false;
    bool                    m_ChangeMode    = false;
    std::list<DisposeItem>  m_Disposer;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    bool Init(uint32_t width, uint32_t height, a3d::RESOURCE_FORMAT colorFormat, a3d::RESOURCE_FORMAT depthFormat);
    void Term();
    void MainLoop();
    void OnEvent(const a3d::MouseEventArg& arg) override;
    void OnEvent(const a3d::ResizeEventArg& arg) override;
    void OnEvent(const a3d::KeyEventArg& arg) override;
    void OnEvent(uint32_t keyCode) override;
    void Draw();
    void FrameDispose();
};
