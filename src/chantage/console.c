#include <windows.h>
#include <chantage/chantage.h>

extern HWND gGameWindow;
static ATOM sConsoleClass;
static HWND sConsoleWindow;

static LRESULT CALLBACK Console_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            // Fill with 50% gray background
            HBRUSH grayBrush = CreateSolidBrush(RGB(128, 128, 128));
            FillRect(hdc, &rect, grayBrush);
            DeleteObject(grayBrush);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN:
        {
            // Handle keyboard input here
            char keyChar = (char)wParam;
            if (keyChar >= 32 && keyChar <= 126) // Printable characters
            {
                // TODO: Add character to input buffer
            }
            else if (wParam == VK_RETURN)
            {
                // TODO: Execute lua command
            }
            else if (wParam == VK_BACK)
            {
                // TODO: Remove last character from input buffer
            }
            return 0;
        }
        case WM_CHAR:
        {
            // Handle character input
            char ch = (char)wParam;
            if (ch >= 32 && ch <= 126) // Printable ASCII
            {
                // TODO: Add to input buffer and redraw
            }
            return 0;
        }
        case WM_DESTROY:
            sConsoleWindow = NULL;
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

static void Console_RegisterClass(void)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = Console_WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "ConsoleClass";
    wc.hbrBackground = NULL; // No automatic background brush
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    sConsoleClass = RegisterClassA(&wc);
}

static void Console_Open(void)
{
    if (!gGameWindow)
        return;

    // Get game window dimensions
    RECT gameRect;
    GetWindowRect(gGameWindow, &gameRect);

    int consoleWidth = (gameRect.right - gameRect.left);
    int consoleHeight = (gameRect.bottom - gameRect.top);
    int consoleX = gameRect.left;
    int consoleY = gameRect.top;

    sConsoleWindow = CreateWindowExA(
        WS_EX_LAYERED,
        "ConsoleClass",
        NULL, // No title
        WS_POPUP, // Borderless
        consoleX, consoleY, consoleWidth, consoleHeight,
        gGameWindow,
        NULL,
        GetModuleHandleA(NULL),
        NULL
    );

    if (sConsoleWindow)
    {
        // Set 50% transparency (128 out of 255)
        SetLayeredWindowAttributes(sConsoleWindow, 0, 128, LWA_ALPHA);
        ShowWindow(sConsoleWindow, SW_SHOW);
        SetFocus(sConsoleWindow); // Steal focus from game
        UpdateWindow(sConsoleWindow);
    }
}

static void Console_Close(void)
{
    if (sConsoleWindow)
    {
        DestroyWindow(sConsoleWindow);
        sConsoleWindow = NULL;

        // Return focus to the game
        if (gGameWindow)
            SetFocus(gGameWindow);
    }
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
            return 1;
        }

        if (sConsoleWindow && IsWindowVisible(sConsoleWindow))
        {
            // Forward the key to the console window
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
            {
                PostMessage(sConsoleWindow, WM_KEYDOWN, p->vkCode, 0);
            }
            return 1;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Init_Console(void)
{
    Console_RegisterClass();
    SetWindowsHookExA(WH_KEYBOARD_LL, ConsoleKeyboardProc, GetModuleHandleA(NULL), 0);
}
