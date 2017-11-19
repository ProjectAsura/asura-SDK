//-------------------------------------------------------------------------------------------------
// File : a3dx_app_win32.cpp
// Desc : Application.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#if defined(WIN32) || defined(_WIN32) || defined(WIN64)

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3dx_app.h>
#include <new>
#include <Windows.h>



namespace {

//-------------------------------------------------------------------------------------------------
// Constant Values
//-------------------------------------------------------------------------------------------------
static const auto WindowClassName = TEXT("a3d_sample");

uint32_t ConvertKey( uint32_t keyCode )
{
    auto key = keyCode;
    auto result = 0u;

    auto isAscii = ( 0x20 <= key && key <= 0x7e );
    if ( isAscii )
    { return key; }

    switch( key )
    {
        case a3d::KEY_RETURN:  { result = VK_RETURN; }     break;
        case a3d::KEY_TAB:     { result = VK_TAB; }        break;
        case a3d::KEY_ESC:     { result = VK_ESCAPE; }     break;
        case a3d::KEY_BACK:    { result = VK_BACK; }       break;
        case a3d::KEY_SHIFT:   { result = VK_SHIFT; }      break;
        case a3d::KEY_CONTROL: { result = VK_CONTROL; }    break;
        case a3d::KEY_ALT:     { result = VK_MENU; }       break;
        case a3d::KEY_F1:      { result = VK_F1; }         break;
        case a3d::KEY_F2:      { result = VK_F2; }         break;
        case a3d::KEY_F3:      { result = VK_F3; }         break;
        case a3d::KEY_F4:      { result = VK_F4; }         break;
        case a3d::KEY_F5:      { result = VK_F5; }         break;
        case a3d::KEY_F6:      { result = VK_F6; }         break;
        case a3d::KEY_F7:      { result = VK_F7; }         break;
        case a3d::KEY_F8:      { result = VK_F8; }         break;
        case a3d::KEY_F9:      { result = VK_F9; }         break;
        case a3d::KEY_F10:     { result = VK_F10; }        break;
        case a3d::KEY_F11:     { result = VK_F11; }        break;
        case a3d::KEY_F12:     { result = VK_F12; }        break;
        case a3d::KEY_UP:      { result = VK_UP; }         break;
        case a3d::KEY_DOWN:    { result = VK_DOWN; }       break;
        case a3d::KEY_LEFT:    { result = VK_LEFT; }       break;
        case a3d::KEY_RIGHT:   { result = VK_RIGHT; }      break;
        case a3d::KEY_NUM0:    { result = VK_NUMPAD0; }    break;
        case a3d::KEY_NUM1:    { result = VK_NUMPAD1; }    break;
        case a3d::KEY_NUM2:    { result = VK_NUMPAD2; }    break;
        case a3d::KEY_NUM3:    { result = VK_NUMPAD3; }    break;
        case a3d::KEY_NUM4:    { result = VK_NUMPAD4; }    break;
        case a3d::KEY_NUM5:    { result = VK_NUMPAD5; }    break;
        case a3d::KEY_NUM6:    { result = VK_NUMPAD6; }    break;
        case a3d::KEY_NUM7:    { result = VK_NUMPAD7; }    break;
        case a3d::KEY_NUM8:    { result = VK_NUMPAD8; }    break;
        case a3d::KEY_NUM9:    { result = VK_NUMPAD9; }    break;
        case a3d::KEY_INSERT:  { result = VK_INSERT; }     break;
        case a3d::KEY_DELETE:  { result = VK_DELETE; }     break;
        case a3d::KEY_HOME:    { result = VK_HOME; }       break;
        case a3d::KEY_END:     { result = VK_END; }        break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// WinApp class
///////////////////////////////////////////////////////////////////////////////////////////////////
class WinApp : public a3d::IApp
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
    // public methods
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //      �R���X�g���N�^�ł�.
    //---------------------------------------------------------------------------------------------
    WinApp()
    : m_hInst           (nullptr)
    , m_hWnd            (nullptr)
    , m_Msg             ()
    , m_Width           (0)
    , m_Height          (0)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //      �f�X�g���N�^�ł�.
    //---------------------------------------------------------------------------------------------
    virtual ~WinApp()
    { Term(); }

    //---------------------------------------------------------------------------------------------
    //      �����������ł�.
    //---------------------------------------------------------------------------------------------
    bool Init
    (
        uint32_t        width,
        uint32_t        height,
        a3d::OnMouse    mouseFunc,
        a3d::OnKey      keyFunc,
        a3d::OnResize   resizeFunc,
        a3d::OnTyping   typingFunc
    )
    {
        // �C���X�^���X�n���h�����擾.
        HINSTANCE hInst = GetModuleHandle( nullptr );
        if ( !hInst )
        { return false; }

        m_Width  = width;
        m_Height = height;

        // �g���E�B���h�E�N���X�̐ݒ�.
        WNDCLASSEX wc;
        wc.cbSize           = sizeof( WNDCLASSEX );
        wc.style            = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc      = WndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = hInst;
        wc.hIcon            = LoadIcon( hInst, IDI_APPLICATION );
        wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
        wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = WindowClassName;
        wc.hIconSm          = LoadIcon( hInst, IDI_APPLICATION );

        // �E�B���h�E�N���X��o�^���܂�.
        if ( !RegisterClassEx( &wc ) )
        { return false; }

        // �C���X�^���X�n���h����ݒ�.
        m_hInst = hInst;

        // ��`�̐ݒ�.
        RECT rc = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

        // �w�肳�ꂽ�N���C�A���g�̈���m�ۂ��邽�߂ɕK�v�ȃE�B���h�E���W���v�Z���܂�.
        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
        AdjustWindowRect( &rc, style, FALSE );

        // �E�B���h�E�𐶐����܂�.
        m_hWnd = CreateWindow(
            WindowClassName,
            WindowClassName,
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            ( rc.right - rc.left ),
            ( rc.bottom - rc.top ),
            NULL,
            NULL,
            hInst,
            this
        );

        // �����`�F�b�N.
        if ( !m_hWnd )
        { return false; }

        m_OnKey     = keyFunc;
        m_OnMouse   = mouseFunc;
        m_OnResize  = resizeFunc;
        m_OnTyping  = typingFunc;

        // �E�B���h�E��\�����܂�.
        ShowWindow( m_hWnd, SW_SHOWNORMAL );
        UpdateWindow( m_hWnd );

        // �t�H�[�J�X��ݒ肵�܂�.
        SetFocus( m_hWnd );

        return true;
    }

    //---------------------------------------------------------------------------------------------
    //      �I�������ł�.
    //---------------------------------------------------------------------------------------------
    void Term()
    {
        if (m_hInst != nullptr)
        {
            UnregisterClass(WindowClassName, m_hInst);
            m_hInst = nullptr;
        }
    }

    //---------------------------------------------------------------------------------------------
    //      ��������ł�.
    //---------------------------------------------------------------------------------------------
    void Release() override
    {
        delete this;
    }

    //---------------------------------------------------------------------------------------------
    //      ���C�����[�v�𑱍s���邩�ǂ���?
    //---------------------------------------------------------------------------------------------
    bool IsLoop() override
    {
        if ( PeekMessage(&m_Msg, nullptr, 0, 0, PM_REMOVE ) != 0 )
        {
            TranslateMessage( &m_Msg );
            DispatchMessage( &m_Msg );
        }

        return m_Msg.message != WM_QUIT;
    }

    //---------------------------------------------------------------------------------------------
    //      �I���v�����o���܂�.
    //---------------------------------------------------------------------------------------------
    void PostQuit() override
    { PostQuitMessage(0); }

    //---------------------------------------------------------------------------------------------
    //      �E�B���h�E�̉������擾���܂�.
    //---------------------------------------------------------------------------------------------
    uint32_t GetWidth() const override
    { return m_Width; }

    //---------------------------------------------------------------------------------------------
    //      �E�B���h�E�̏c�����擾���܂�.
    //---------------------------------------------------------------------------------------------
    uint32_t GetHeight() const override
    { return m_Height; }

    //---------------------------------------------------------------------------------------------
    //      �C���X�^���X�n���h�����擾���܂�.
    //---------------------------------------------------------------------------------------------
    void* GetInstanceHandle() const override
    { return reinterpret_cast<void*>(m_hInst); }

    //---------------------------------------------------------------------------------------------
    //      �E�B���h�E�n���h�����擾���܂�.
    //---------------------------------------------------------------------------------------------
    void* GetWindowHandle() const override
    { return reinterpret_cast<void*>(m_hWnd); }


private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    HINSTANCE           m_hInst         = nullptr;  //!< �C���X�^���X�n���h���ł�.
    HWND                m_hWnd          = nullptr;  //!< �E�B���h�E�n���h���ł�.
    MSG                 m_Msg           = {};       //!< ���b�Z�[�W�ł�.
    uint32_t            m_Width         = 0;        //!< �E�B���h�E�̉����ł�.
    uint32_t            m_Height        = 0;        //!< �E�B���h�E�̏c���ł�.
    a3d::OnKey          m_OnKey         = nullptr;  //!< �L�[�{�[�h�R�[���o�b�N.
    a3d::OnMouse        m_OnMouse       = nullptr;  //!< �}�E�X�R�[���o�b�N.
    a3d::OnResize       m_OnResize      = nullptr;  //!< ���T�C�Y�R�[���o�b�N.
    a3d::OnTyping       m_OnTyping      = nullptr;  //!< �^�C�s���O�R�[���o�b�N.
    a3d::IAllocator*    m_pAllocator    = nullptr;  

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //      �E�B���h�E�v���V�[�W���ł�.
    //---------------------------------------------------------------------------------------------
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto instance = reinterpret_cast<WinApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        const UINT OLD_WM_MOUSEWHEEL = 0x020A;

        if (msg == WM_KEYDOWN    ||
            msg == WM_SYSKEYDOWN ||
            msg == WM_KEYUP      ||
            msg == WM_SYSKEYUP)
        {
            // ESC�L�[�������ꂽ��I������.
            if ( wp == 0x1b )
            {
                PostQuitMessage(0);
                return 0;
            }

            if ( instance->m_OnKey == nullptr)
            { return 0; }

            auto isKeyDown = ( msg == WM_KEYDOWN ) || ( msg == WM_SYSKEYDOWN );
            uint32_t mask  = ( 1 << 29 );
            auto isAltDown = ( ( lp & mask ) != 0 );

            a3d::KeyEventArg arg = {};
            arg.IsKeyDown = isKeyDown;
            arg.IsAltDown = isAltDown;
            arg.KeyCode   = ConvertKey( uint32_t(wp) );

            instance->m_OnKey(arg);
        }

        if (msg == WM_LBUTTONDOWN   ||
            msg == WM_LBUTTONUP     ||
            msg == WM_LBUTTONDBLCLK ||
            msg == WM_MBUTTONDOWN   ||
            msg == WM_MBUTTONUP     ||
            msg == WM_MBUTTONDBLCLK ||
            msg == WM_RBUTTONDOWN   ||
            msg == WM_RBUTTONUP     ||
            msg == WM_RBUTTONDBLCLK ||
            msg == WM_MOUSEWHEEL    ||
            msg == WM_MOUSEMOVE     ||
            msg == WM_XBUTTONDOWN   ||
            msg == WM_XBUTTONUP     ||
            msg == WM_XBUTTONDBLCLK)
        {
            if ( instance->m_OnMouse == nullptr )
            { return 0; }

            auto x = static_cast<int>( (int16_t)LOWORD( lp ) );
            auto y = static_cast<int>( (int16_t)HIWORD( lp ) );
            auto wheelDelta = 0;

            if ( msg == WM_MOUSEHWHEEL )
            {
                wheelDelta += static_cast<int16_t>( wp );
            }

            auto state = LOWORD( wp );
            auto isDownL  = ( ( state & MK_LBUTTON )  != 0 );
            auto isDownR  = ( ( state & MK_RBUTTON )  != 0 );
            auto isDownM  = ( ( state & MK_MBUTTON )  != 0 );
            //auto isDownX1 = ( ( state & MK_XBUTTON1 ) != 0 );
            //auto isDownX2 = ( ( state & MK_XBUTTON2 ) != 0 );

            a3d::MouseEventArg arg = {};
            arg.CursorY     = x;
            arg.CursorY     = y;
            arg.WheelDelta  = wheelDelta;
            arg.IsDownL     = isDownL;
            arg.IsDownM     = isDownM;
            arg.IsDownR     = isDownR;

            instance->m_OnMouse(arg);
        }

        switch( msg )
        {
            case WM_CREATE:
            {
                auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lp);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            }
            break;

        case WM_DESTROY:
            { PostQuitMessage(0); }
            break;

        case WM_CLOSE:
            { DestroyWindow(hWnd); }
            break;

        case WM_CHAR:
            {
                if ( instance->m_OnTyping == nullptr )
                { return 0; }

                auto keyCode = ConvertKey( uint32_t(wp) );
                instance->m_OnTyping( keyCode );
            }
            break;

        case WM_SIZE:
            {
                auto w = static_cast<uint32_t>(LOWORD(lp));
                auto h = static_cast<uint32_t>(HIWORD(lp));

                instance->m_Width  = w;
                instance->m_Height = h;
                
                if ( instance->m_OnResize == nullptr )
                { return 0; }

                a3d::ResizeEventArg arg = {};
                arg.Width       = w;
                arg.Height      = h;
                arg.AspectRatio = float(w) / float(h);

                instance->m_OnResize( arg );
            }
            break;

        default:
            break;
        }

        return DefWindowProc(hWnd, msg, wp, lp);
    }
};

} // namespace

namespace a3d {

//-------------------------------------------------------------------------------------------------
//      �A�v���P�[�V�����𐶐����܂�.
//-------------------------------------------------------------------------------------------------
bool CreateApp
(
    IAllocator* pAllocator,
    uint32_t    width,
    uint32_t    height,
    OnMouse     mouseFunc,
    OnKey       keyFunc,
    OnResize    resizeFunc,
    OnTyping    typingFunc,
    IApp**      ppApp
)
{
    if (width == 0 && height == 0)
    { return false; }

    if (ppApp == nullptr)
    { return false; }

    auto buf = pAllocator->Alloc(sizeof(WinApp), alignof(WinApp));
    auto instance = new(buf) WinApp();
    if (!instance->Init(width, height, mouseFunc, keyFunc, resizeFunc, typingFunc))
    {
        instance->Release();
        return false;
    }

    *ppApp = instance;
    return true;
}

} // namespace a3d


#endif// defined(WIN32) || defined(_WIN32) || defined(WIN64)
