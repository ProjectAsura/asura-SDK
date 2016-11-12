//-------------------------------------------------------------------------------------------------
// File : SampleAppWin.cpp
// Desc : Sample Application Implementation for Windows Platform.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#if defined(WIN32) || defined(_WIN32) || defined(WIN64)

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "SampleApp.h"
#include <Windows.h>


namespace {

//-------------------------------------------------------------------------------------------------
// Global variables.
//-------------------------------------------------------------------------------------------------
const auto WindowClassName = TEXT("A3D_Sample");


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////
class App : public IApp
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
    App()
    : m_hInst           (nullptr)
    , m_hWnd            (nullptr)
    , m_Msg             ()
    , m_Width           (0)
    , m_Height          (0)
    , m_OnMouse         (nullptr)
    , m_pUserMouse      (nullptr)
    , m_OnKeyborad      (nullptr)
    , m_pUserKeyboard   (nullptr)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //      �f�X�g���N�^�ł�.
    //---------------------------------------------------------------------------------------------
    virtual ~App()
    { Term(); }

    //---------------------------------------------------------------------------------------------
    //      �����������ł�.
    //---------------------------------------------------------------------------------------------
    bool Init(uint32_t width, uint32_t height)
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
        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
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
    { delete this; }

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

    //---------------------------------------------------------------------------------------------
    //      �}�E�X�R�[���o�b�N�֐���ݒ肵�܂�.
    //---------------------------------------------------------------------------------------------
    void SetMouseCallback(MouseCallback pFunc, void* pUser) override
    {
        m_OnMouse    = pFunc;
        m_pUserMouse = pUser;
    }

    //---------------------------------------------------------------------------------------------
    //      �L�[�{�[�h�R�[���o�b�N�֐���ݒ肵�܂�.
    //---------------------------------------------------------------------------------------------
    void SetKeyboardCallback(KeyboardCallback pFunc, void* pUser) override
    {
        m_OnKeyborad    = pFunc;
        m_pUserKeyboard = pUser;
    }

    //---------------------------------------------------------------------------------------------
    //      �������̓R�[���o�b�N�֐���ݒ肵�܂�.
    //---------------------------------------------------------------------------------------------
    void SetCharCallback(CharCallback pFunc, void* pUser) override
    {
        m_OnChar    = pFunc;
        m_pUserChar = pUser;
    }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    HINSTANCE   m_hInst;        //!< �C���X�^���X�n���h���ł�.
    HWND        m_hWnd;         //!< �E�B���h�E�n���h���ł�.
    MSG         m_Msg;          //!< ���b�Z�[�W�ł�.
    uint32_t    m_Width;        //!< �E�B���h�E�̉����ł�.
    uint32_t    m_Height;       //!< �E�B���h�E�̏c���ł�.

    MouseCallback       m_OnMouse;          //!< �}�E�X�R�[���o�b�N�֐��ł�.
    void*               m_pUserMouse;       //!< �}�E�X�R�[���o�b�N�̃��[�U�[�f�[�^�ł�.
    KeyboardCallback    m_OnKeyborad;       //!< �L�[�{�[�h�R�[���o�b�N�֐��ł�.
    void*               m_pUserKeyboard;    //!< �L�[�{�[�h�R�[���o�b�N�̃��[�U�[�f�[�^�ł�.
    CharCallback        m_OnChar;           //!< �������̓R�[���o�b�N�֐��ł�.
    void*               m_pUserChar;        //!< �������̓R�[���o�b�N�̃��[�U�[�f�[�^�ł�.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //      �E�B���h�E�v���V�[�W���ł�.
    //---------------------------------------------------------------------------------------------
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto instance = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        const UINT OLD_WM_MOUSEWHEEL = 0x020A;

        if (msg == WM_KEYDOWN    ||
            msg == WM_SYSKEYDOWN ||
            msg == WM_KEYUP      ||
            msg == WM_SYSKEYUP)
        {
            if ( instance->m_OnKeyborad == nullptr)
            { return 0; }

            auto isKeyDown = ( msg == WM_KEYDOWN ) || ( msg == WM_SYSKEYDOWN );
            uint32_t mask  = ( 1 << 29 );
            auto isAltDown = ( ( lp & mask ) != 0 );

            instance->m_OnKeyborad(isKeyDown, isAltDown, uint32_t(wp), instance->m_pUserKeyboard);
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

            instance->m_OnMouse(x, y, wheelDelta, isDownL, isDownM, isDownR, instance->m_pUserMouse);
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
                if ( instance->m_OnChar == nullptr )
                { return 0; }

                instance->m_OnChar( uint32_t(wp), instance->m_pUserChar );
            }
            break;;

        default:
            break;
        }

        return DefWindowProc(hWnd, msg, wp, lp);
    }
};

} // namespace


//-------------------------------------------------------------------------------------------------
//      �A�v���P�[�V�����𐶐����܂�.
//-------------------------------------------------------------------------------------------------
bool CreateApp(uint32_t width, uint32_t height, IApp** ppApp)
{
    if (width == 0 && height == 0)
    { return false; }

    if (ppApp == nullptr)
    { return false; }

    auto instance = new App;
    if (!instance->Init(width, height))
    {
        instance->Release();
        return false;
    }

    *ppApp = instance;
    return true;
}

#endif// defined(WIN32) || defined(_WIN32) || defined(WIN64)
