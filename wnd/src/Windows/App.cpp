//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "App.h"


namespace {

//-------------------------------------------------------------------------------------------------
// Global variables.
//-------------------------------------------------------------------------------------------------
const auto WindowClassName = ("wnd");


//-------------------------------------------------------------------------------------------------
//      キーコードを変換します.
//-------------------------------------------------------------------------------------------------
uint32_t ConvertKeyCode(uint32_t keyCode)
{
    auto key = keyCode;
    auto result = 0u;

    bool isAscii = ( 0x20 <= key && key <= 0x7e );
    if ( isAscii )
    { return key; }

    switch( key )
    {
        case VK_RETURN:  { result = wnd::KEY_RETURN; }  break;
        case VK_TAB:     { result = wnd::KEY_TAB; }     break;
        case VK_ESCAPE:  { result = wnd::KEY_ESC; }     break;
        case VK_BACK:    { result = wnd::KEY_BACK; }    break;
        case VK_SHIFT:   { result = wnd::KEY_SHIFT; }   break;
        case VK_CONTROL: { result = wnd::KEY_CONTROL; } break;
        case VK_MENU:    { result = wnd::KEY_ALT; }     break;
        case VK_F1:      { result = wnd::KEY_F1; }      break;
        case VK_F2:      { result = wnd::KEY_F2; }      break;
        case VK_F3:      { result = wnd::KEY_F3; }      break;
        case VK_F4:      { result = wnd::KEY_F4; }      break;
        case VK_F5:      { result = wnd::KEY_F5; }      break;
        case VK_F6:      { result = wnd::KEY_F6; }      break;
        case VK_F7:      { result = wnd::KEY_F7; }      break;
        case VK_F8:      { result = wnd::KEY_F8; }      break;
        case VK_F9:      { result = wnd::KEY_F9; }      break;
        case VK_F10:     { result = wnd::KEY_F10; }     break;
        case VK_F11:     { result = wnd::KEY_F11; }     break;
        case VK_F12:     { result = wnd::KEY_F12; }     break;
        case VK_UP:      { result = wnd::KEY_UP; }      break;
        case VK_DOWN:    { result = wnd::KEY_DOWN; }    break;
        case VK_LEFT:    { result = wnd::KEY_LEFT; }    break;
        case VK_RIGHT:   { result = wnd::KEY_RIGHT; }   break;
        case VK_NUMPAD0: { result = wnd::KEY_NUM0; }    break;
        case VK_NUMPAD1: { result = wnd::KEY_NUM1; }    break;
        case VK_NUMPAD2: { result = wnd::KEY_NUM2; }    break;
        case VK_NUMPAD3: { result = wnd::KEY_NUM3; }    break;
        case VK_NUMPAD4: { result = wnd::KEY_NUM4; }    break;
        case VK_NUMPAD5: { result = wnd::KEY_NUM5; }    break;
        case VK_NUMPAD6: { result = wnd::KEY_NUM6; }    break;
        case VK_NUMPAD7: { result = wnd::KEY_NUM7; }    break;
        case VK_NUMPAD8: { result = wnd::KEY_NUM8; }    break;
        case VK_NUMPAD9: { result = wnd::KEY_NUM9; }    break;
        case VK_INSERT:  { result = wnd::KEY_INSERT; }  break;
        case VK_DELETE:  { result = wnd::KEY_DELETE; }  break;
        case VK_HOME:    { result = wnd::KEY_HOME; }    break;
        case VK_END:     { result = wnd::KEY_END; }     break;
    }

    return result;
}

} // namespace 


namespace wnd {

///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
App::App()
: m_RefCount        (1)
, m_hInstance       (nullptr)
, m_hWnd            (nullptr)
, m_Width           (0)
, m_Height          (0)
, m_AspectRatio     (1.0f)
, m_MouseHandler    (nullptr)
, m_KeyboardHandler (nullptr)
, m_TypingHandler   (nullptr)
, m_ResizeHandler   (nullptr)
, m_pUserMouse      (nullptr)
, m_pUserKeyboard   (nullptr)
, m_pUserTyping     (nullptr)
, m_pUserResize     (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool App::Init(const char* title, uint32_t width, uint32_t height)
{
    strcpy_s(m_Title, title);
    // インスタンスハンドルを取得.
    HINSTANCE hInst = GetModuleHandleA( nullptr );
    if ( !hInst )
    { return false; }

    m_Width  = width;
    m_Height = height;
    m_AspectRatio = float(width) / float(height);

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEXA wc;
    wc.cbSize           = sizeof( WNDCLASSEXA );
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIconA( hInst, (LPCSTR)IDI_APPLICATION );
    wc.hCursor          = LoadCursorA( NULL, (LPSTR)IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = WindowClassName;
    wc.hIconSm          = LoadIconA( hInst, (LPSTR)IDI_APPLICATION );

    // ウィンドウクラスを登録します.
    if ( !RegisterClassExA( &wc ) )
    { return false; }

    // インスタンスハンドルを設定.
    m_hInstance = hInst;

    // 矩形の設定.
    RECT rc = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成します.
    m_hWnd = CreateWindowA(
        WindowClassName,
        m_Title,
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

    // 生成チェック.
    if ( !m_hWnd )
    { return false; }

    // ウィンドウを表示します.
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカスを設定します.
    SetFocus( m_hWnd );

    return true;

}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void App::Term()
{
    if (m_hInstance != nullptr)
    {
        UnregisterClassA(WindowClassName, m_hInstance);
        m_hInstance = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void App::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void App::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t App::GetCount() const 
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      メインループを続行するかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool App::IsLoop()
{
    if ( PeekMessageA(&m_Msg, nullptr, 0, 0, PM_REMOVE ) != 0 )
    {
        TranslateMessage( &m_Msg );
        DispatchMessageA( &m_Msg );
    }

    return m_Msg.message != WM_QUIT;
}

//-------------------------------------------------------------------------------------------------
//      終了要求を通知します.
//-------------------------------------------------------------------------------------------------
void App::RequestQuit()
{ PostQuitMessage(0); }

//-------------------------------------------------------------------------------------------------
//      ウィンドウの横幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t App::GetWidth() const
{ return m_Width; }

//-------------------------------------------------------------------------------------------------
//      ウィンドウの縦幅を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t App::GetHeight() const
{ return m_Height; }

//-------------------------------------------------------------------------------------------------
//      ウィンドウのアスペクト比を取得します.
//-------------------------------------------------------------------------------------------------
float App::GetAspectRatio() const
{ return m_AspectRatio; }

//-------------------------------------------------------------------------------------------------
//      インスタンスハンドルを取得します.
//-------------------------------------------------------------------------------------------------
void* App::GetInstanceHandle() const
{ return reinterpret_cast<void*>(m_hInstance); }

//-------------------------------------------------------------------------------------------------
//      ウィンドウハンドルを取得します.
//-------------------------------------------------------------------------------------------------
void* App::GetWindowHandle() const
{ return reinterpret_cast<void*>(m_hWnd); }

//-------------------------------------------------------------------------------------------------
//      マウスハンドラーを設定します.
//-------------------------------------------------------------------------------------------------
void App::SetMouseHandler(MouseHandler handler, void* pUser)
{
    m_MouseHandler = handler;
    m_pUserMouse   = pUser;
}

//-------------------------------------------------------------------------------------------------
//      キーボードハンドラーを設定します.
//-------------------------------------------------------------------------------------------------
void App::SetKeyboardHandler(KeyboardHandler handler, void* pUser)
{
    m_KeyboardHandler = handler;
    m_pUserKeyboard   = pUser;
}

//-------------------------------------------------------------------------------------------------
//      タイピングハンドラーを設定します.
//-------------------------------------------------------------------------------------------------
void App::SetTypingHandler(TypingHandler handler, void* pUser)
{
    m_TypingHandler = handler;
    m_pUserTyping   = pUser;
}

//-------------------------------------------------------------------------------------------------
//      リサイズハンドラーを設定します.
//-------------------------------------------------------------------------------------------------
void App::SetResizeHandler(ResizeHandler handler, void* pUser)
{
    m_ResizeHandler = handler;
    m_pUserResize   = pUser;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウプロシージャを設定します.
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto instance = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    const UINT OLD_WM_MOUSEWHEEL = 0x020A;

    if (msg == WM_KEYDOWN    ||
        msg == WM_SYSKEYDOWN ||
        msg == WM_KEYUP      ||
        msg == WM_SYSKEYUP)
    {
        // ESCキーが押されたら終了する.
        if ( wp == 0x1b && instance->m_KeyboardHandler == nullptr)
        {
            PostQuitMessage(0);
            return 0;
        }

        if ( instance->m_KeyboardHandler == nullptr)
        { return 0; }

        auto isKeyDown = ( msg == WM_KEYDOWN ) || ( msg == WM_SYSKEYDOWN );
        uint32_t mask  = ( 1 << 29 );
        auto isAltDown = ( ( lp & mask ) != 0 );
        auto keyCode = ConvertKeyCode( uint32_t(wp) );

        KeyEvent args = {};
        args.IsAltDown = isAltDown;
        args.IsKeyDown = isKeyDown;
        args.KeyCode   = keyCode;

        instance->m_KeyboardHandler(args, instance->m_pUserKeyboard);
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
        if ( instance->m_MouseHandler == nullptr )
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
        auto isDownX1 = ( ( state & MK_XBUTTON1 ) != 0 );
        auto isDownX2 = ( ( state & MK_XBUTTON2 ) != 0 );

        MouseEvent args = {};
        args.CursorX                 = x;
        args.CursorY                 = y;
        args.WheelDelta              = wheelDelta;
        args.IsDown[MOUSE_BUTTON_L]  = isDownL;
        args.IsDown[MOUSE_BUTTON_R]  = isDownR;
        args.IsDown[MOUSE_BUTTON_M]  = isDownM;
        args.IsDown[MOUSE_BUTTON_X1] = isDownX1;
        args.IsDown[MOUSE_BUTTON_X2] = isDownX2;

        instance->m_MouseHandler(args, instance->m_pUserMouse);
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
            if ( instance->m_TypingHandler == nullptr )
            { return 0; }

            TypingEvent args = {};
            args.KeyCode = ConvertKeyCode( uint32_t(wp) );

            instance->m_TypingHandler( args, instance->m_pUserTyping );
        }
        break;

    case WM_SIZE:
        {
            auto w = static_cast<uint32_t>(LOWORD( lp ));
            auto h = static_cast<uint32_t>(HIWORD( lp ));
            auto a = float(w) / float(h);

            instance->m_Width       = w;
            instance->m_Height      = h;
            instance->m_AspectRatio = a;

            if ( instance->m_ResizeHandler == nullptr )
            { return 0; }

            ResizeEvent args = {};
            args.Width       = w;
            args.Height      = h;
            args.AspectRatio = a;

            instance->m_ResizeHandler( args, instance->m_pUserResize );
        }
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
}

} // namespace wnd
