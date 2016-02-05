
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "platform.h"
#include "../input.h"
#include <assert.h>

static HDC s_device_context;
static HGLRC s_gl_context;
static HWND s_hWnd;

LRESULT CALLBACK MessageLoop(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch(uMsg)
    {
    case WM_CREATE:
    {
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
            PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
            32,                        //Colordepth of the framebuffer.
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            0, //24,                  //Number of bits for the depthbuffer
            0,                        //Number of bits for the stencilbuffer
            0,                        //Number of Aux buffers in the framebuffer.
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        s_device_context = GetDC(hWnd);
        int pixel_format = ChoosePixelFormat(s_device_context, &pfd);
        SetPixelFormat(s_device_context, pixel_format, &pfd);

        s_gl_context = wglCreateContext(s_device_context);
        wglMakeCurrent(s_device_context, s_gl_context);

    }break;
    case WM_DESTROY:
    {
        wglMakeCurrent(s_device_context, NULL);
        wglDeleteContext(s_gl_context);

    }break;
    case WM_CLOSE:
    {
        PostQuitMessage(1);
    }break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
        {
            PostQuitMessage(1);
        }break;
        }
    }break;
    case WM_MOUSEMOVE:
    {
        result = 0;
    }break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_OWNDC;
    window_class.lpszClassName = "EnGenWindowClass";
    window_class.lpfnWndProc = (WNDPROC) MessageLoop;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    if (!RegisterClassEx(&window_class))
    {
        return 1;
    }

    DWORD screen_res_x = GetSystemMetrics(SM_CXSCREEN);
    DWORD screen_res_y = GetSystemMetrics(SM_CYSCREEN);

    DWORD window_res_x = (DWORD) (screen_res_x * 0.9f);
    DWORD window_res_y = (DWORD) (screen_res_y * 0.9f);

    DWORD window_x = (screen_res_x - window_res_x) / 2;
    DWORD window_y = (screen_res_y - window_res_y) / 2;

    s_hWnd = CreateWindowEx(
            NULL,                               // dwExStyle,
            window_class.lpszClassName,         // lpClassName,
            "Hello Windows!",                   // lpWindowName,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // dwStyle,
            window_x,                           // x,
            window_y,                           // y,
            window_res_x,                       // nWidth,
            window_res_y,                       // nHeight,
            NULL,                               // hWndParent,
            NULL,                               // hMenu,
            hInstance,                          // hInstance,
            NULL);                              // lpParam);


    int game_result = GameMain();
    return game_result;
}

static KeyCode VKKeyToKeyCode(WPARAM p)
{
    uint32 key_code = (uint32)p;
    KeyCode result = KeyCode_UNKNOWN;

#define MAP_KEY(vk, res) case vk: result = res; break

    switch (key_code)
    {
    MAP_KEY(VK_BACK     , KeyCode_BACKSPACE);
    MAP_KEY(VK_TAB      , KeyCode_TAB);
    MAP_KEY(VK_RETURN   , KeyCode_ENTER);
    MAP_KEY(VK_SHIFT    , KeyCode_SHIFT);
    MAP_KEY(VK_CONTROL  , KeyCode_CONTROL);
    MAP_KEY(VK_MENU     , KeyCode_ALT);
    MAP_KEY(VK_CAPITAL  , KeyCode_CAPSLOCK);
    MAP_KEY(VK_ESCAPE   , KeyCode_ESCAPE);
    MAP_KEY(VK_SPACE    , KeyCode_SPACE);
    MAP_KEY(VK_LEFT     , KeyCode_LEFT);
    MAP_KEY(VK_UP       , KeyCode_UP);
    MAP_KEY(VK_RIGHT    , KeyCode_RIGHT);
    MAP_KEY(VK_DOWN     , KeyCode_DOWN);
    MAP_KEY(VK_OEM_1    , KeyCode_SEMICOLON);
    MAP_KEY(VK_OEM_3    , KeyCode_BACKQUOTE);

#undef MAP_KEY

    default:
    {
        if (key_code >= 'A' && key_code < 'Z')
        {
            result = (KeyCode)(KeyCode_a + (key_code - 'A'));
        }
        else if (key_code >= 0x30 && key_code <= 0x39)
        {
            result = (KeyCode)(KeyCode_0 + (key_code - 0x30));
        }
        else if (key_code >= VK_F1 && key_code <= VK_F12)
        {
            result = (KeyCode)(KeyCode_F1 + (key_code - VK_F1));
        }
        else
        {
            OutputDebugString("Unhandled Key Press!!!");
            //assert(!"Unmapped key press!");
        }
    } break;
    }

    //OutputDebugString("Pressed A Key!\n");

    return result;
}

// TODO: Build input string for text input
bool Platform_RunMessageLoop(NewInput* input)
{
    MSG msg = { 0 };
    assert(s_hWnd);

    bool result = true;

    BeginMessageLoop(input);

    while (PeekMessage(&msg, s_hWnd, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
        case WM_QUIT:
        {
            result = false;
        }break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            KeyCode key = VKKeyToKeyCode(msg.wParam);
            _ProcessKeyboardMessage(input, key, true);
        }break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            KeyCode key = VKKeyToKeyCode(msg.wParam);
            _ProcessKeyboardMessage(input, key, false);
        }break;

        case WM_LBUTTONDOWN:
        {
            _ProcessMouseButtonMessage(input, MouseButton_LEFT, true);
        }break;
        case WM_RBUTTONDOWN:
        {
            _ProcessMouseButtonMessage(input, MouseButton_RIGHT, true);
        }break;
        case WM_MBUTTONDOWN:
        {
            _ProcessMouseButtonMessage(input, MouseButton_MIDDLE, true);
        }break;
        case WM_XBUTTONDOWN:
        {
            MouseButton mouse_button;
            if(msg.wParam & 0x00010000) // XBUTTON1
            {
                mouse_button = MouseButton_FOUR;
            }
            else if(msg.wParam & 0x00020000)
            {
                mouse_button = MouseButton_FIVE;
            }
            else { assert(!"What button did you say again?"); break; }
            _ProcessMouseButtonMessage(input, mouse_button, true);
        }break;

        case WM_LBUTTONUP:
        {
            _ProcessMouseButtonMessage(input, MouseButton_LEFT, false);
        }break;
        case WM_RBUTTONUP:
        {
            _ProcessMouseButtonMessage(input, MouseButton_RIGHT, false);
        }break;
        case WM_MBUTTONUP:
        {
            _ProcessMouseButtonMessage(input, MouseButton_MIDDLE, false);
        }break;
        case WM_XBUTTONUP:
        {
            MouseButton mouse_button;
            if(msg.wParam & 0x00010000) // XBUTTON1
            {
                mouse_button = MouseButton_FOUR;
            }
            else if(msg.wParam & 0x00020000) // XBUTTON2
            {
                mouse_button = MouseButton_FIVE;
            }
            else { assert(!"What button did you say again?"); break; }
            _ProcessMouseButtonMessage(input, mouse_button, false);
        }break;

        case WM_MOUSEMOVE:
        {
            // 0, 0 is lower left corner of the screen
            RECT rect;
            GetClientRect(s_hWnd, &rect);
            input->mouse_state.new_position.x = GET_X_LPARAM(msg.lParam);
            input->mouse_state.new_position.y = rect.bottom - GET_Y_LPARAM(msg.lParam);
        }break;

        default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            break;
        }
    }

    return result;
}

void Platform_SwapBuffers()
{
    SwapBuffers(s_device_context);
}

void GetDrawableSize(int* x, int* y)
{
    RECT rect = {};
    GetClientRect(s_hWnd, &rect);
    *x = rect.right - rect.left;
    *y = rect.top - rect.bottom;
}

Vec2i Platform_GetResolution()
{
    RECT rect = {};
    GetClientRect(s_hWnd, &rect);
    return { rect.right, rect.bottom } ;
}

uint64 Platform_GetCycleCount()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return (uint64)result.QuadPart;
}

uint64 Platform_GetCyclesPerSecond()
{
    static LARGE_INTEGER result = (QueryPerformanceFrequency(&result), result);
    return (uint64)result.QuadPart;
}

uint32 Platform_GetTickCount()
{
    static const uint64 start_cycle_count = Platform_GetCycleCount();
    static const uint64 divisor = (Platform_GetCyclesPerSecond() / 1000);

    uint64 cycle_count = Platform_GetCycleCount() - start_cycle_count;
    cycle_count /= divisor;

    uint32 result = (uint32)(cycle_count & 0xFFFFFFFF);

    return result;
}


