#include <windows.h>
#include <chantage/chantage.h>

extern HWND gGameWindow;
static ATOM sConsoleClass;
static HWND sConsoleWindow;

static void Console_RegisterClass(void)
{
    /* Transparent window */
    WNDCLASS wc = {0};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "ConsoleClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    sConsoleClass = RegisterClassA(&wc);
}

static void Console_Open(void)
{
    if (!gGameWindow)
        return;

    sConsoleWindow = CreateWindowExA(
        WS_EX_TOPMOST,
        "ConsoleClass",
        "Lua Console",
        WS_OVERLAPPEDWINDOW,
        100, 100, 800, 600,
        gGameWindow,
        NULL,
        GetModuleHandleA(NULL),
        NULL
    );

    ShowWindow(sConsoleWindow, SW_SHOW);
}

static void Console_Close(void)
{
    DestroyWindow(sConsoleWindow);
    sConsoleWindow = NULL;
}

void Console_Toggle(void)
{
    if (sConsoleWindow)
        Console_Close();
    else
        Console_Open();
}

LRESULT CALLBACK ConsoleKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN && p->vkCode == VK_OEM_3)
        {
            Console_Toggle();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Init_Console(void)
{
    Console_RegisterClass();
    SetWindowsHookExA(WH_KEYBOARD_LL, ConsoleKeyboardProc, GetModuleHandleA(NULL), 0);
}
