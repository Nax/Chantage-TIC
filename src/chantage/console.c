#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <chantage/chantage.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>

#define MAX_LINES 200
#define LINE_EDIT_BUFFER_SIZE 1024

extern lua_State* gLuaState;
extern HWND gGameWindow;
static ATOM sConsoleClass;
static HWND sConsoleWindow;

static PWCH sLastLines[MAX_LINES];
static WCHAR sLineEditBuffer[LINE_EDIT_BUFFER_SIZE];
static int sLineEditLength = 0;
static BYTE sKeyboardState[256] = {0};

void Console_WriteLineWide(const WCHAR* line)
{
    /* Free the oldest line */
    if (sLastLines[0])
    {
        HeapFree(GetProcessHeap(), 0, sLastLines[0]);
    }

    /* Shift lines up (oldest gets removed, everything moves up) */
    for (int i = 0; i < MAX_LINES - 1; ++i)
    {
        sLastLines[i] = sLastLines[i + 1];
    }

    /* Add new line at the bottom */
    size_t len = wcslen(line) + 1;
    sLastLines[MAX_LINES - 1] = HeapAlloc(GetProcessHeap(), 0, len * 2);
    memcpy(sLastLines[MAX_LINES - 1], line, len * 2);

    /* Redraw console window if visible */
    if (sConsoleWindow && IsWindowVisible(sConsoleWindow))
    {
        InvalidateRect(sConsoleWindow, NULL, TRUE);
        UpdateWindow(sConsoleWindow);
    }
}

static void Console_Exec(const WCHAR* line)
{
    static char sBuffer[4096];
    static WCHAR sBufferWide[1024];

    /* Try to compile as 'return ' + expr */
    strcpy(sBuffer, "return ");
    int prefixLen = (int)strlen(sBuffer);
    int exprLen = WideCharToMultiByte(CP_UTF8, 0, line, (int)wcslen(line), sBuffer + prefixLen, sizeof(sBuffer) - prefixLen - 1, NULL, NULL);
    sBuffer[prefixLen + exprLen] = '\0';

    /* Load the string */
    if (luaL_loadstring(gLuaState, sBuffer) != LUA_OK)
    {
        /* Pop error */
        lua_pop(gLuaState, 1);

        /* Fallback to normal execution */
        int normalLen = WideCharToMultiByte(CP_UTF8, 0, line, (int)wcslen(line), sBuffer, sizeof(sBuffer) - 1, NULL, NULL);
        sBuffer[normalLen] = '\0';
        if (luaL_loadstring(gLuaState, sBuffer) != LUA_OK)
        {
            const char* errorMsg = lua_tostring(gLuaState, -1);
            swprintf(sBufferWide, 1024, L"Lua Error: %S", errorMsg);
            Console_WriteLineWide(sBufferWide);
            lua_pop(gLuaState, 1);
            return;
        }
    }

    /* We have some valid lua, execute */
    if (lua_pcall(gLuaState, 0, LUA_MULTRET, 0) != LUA_OK)
    {
        const char* errorMsg = lua_tostring(gLuaState, -1);
        swprintf(sBufferWide, 1024, L"Lua Error: %S", errorMsg);
        Console_WriteLineWide(sBufferWide);
        lua_pop(gLuaState, 1);
        return;
    }

    /* Handle return values */
    int returnCount = lua_gettop(gLuaState);
    if (returnCount > 0)
    {
        for (int i = 1; i <= returnCount; ++i)
        {
            size_t len;
            const char* returnStr = luaL_tolstring(gLuaState, i, &len);
            if (returnStr)
            {
                swprintf(sBufferWide, 1024, L"%S", returnStr);
                Console_WriteLineWide(sBufferWide);
            }
            lua_pop(gLuaState, 1);
        }
    }

    lua_settop(gLuaState, 0);
}

static void Console_Run(void)
{
    /* Check for empty command */
    if (!sLineEditLength)
        return;

    /* Check for all whitespace */
    for (int i = 0; i < sLineEditLength; ++i)
    {
        if (sLineEditBuffer[i] != ' ' && sLineEditBuffer[i] != '\t' && sLineEditBuffer[i] != '\n' && sLineEditBuffer[i] != '\r')
            break;
        if (i == sLineEditLength - 1)
        {
            // All whitespace
            sLineEditLength = 0;
            sLineEditBuffer[0] = '\0';
            return;
        }
    }

    /* Write the command */
    Console_WriteLineWide(sLineEditBuffer);

    /* Execute */
    Console_Exec(sLineEditBuffer);

    /* Clear input buffer */
    sLineEditLength = 0;
    sLineEditBuffer[0] = '\0';
}

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

            /* Background */
            HBRUSH grayBrush = CreateSolidBrush(RGB(128, 128, 128));
            FillRect(hdc, &rect, grayBrush);
            DeleteObject(grayBrush);

            /* Text */
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));

            /* Draw lines: anchor at top if few lines, scroll from bottom if many */
            int lineHeight = 16; // Approximate line height
            int maxDisplayLines = (rect.bottom - rect.top) / lineHeight - 1;

            // Count actual lines
            int actualLineCount = 0;
            for (int i = 0; i < MAX_LINES; ++i)
            {
                if (sLastLines[i])
                    actualLineCount++;
            }

            // If we have fewer lines than can fit, start from 0 (anchor at top)
            // Otherwise, show only the most recent lines that fit
            int startLine = 0;
            if (actualLineCount > maxDisplayLines)
            {
                startLine = MAX_LINES - maxDisplayLines;
            }

            int displayY = 0;
            for (int i = startLine; i < MAX_LINES; ++i)
            {
                if (sLastLines[i])
                {
                    TextOutW(hdc, 5, displayY, sLastLines[i], (int)wcslen(sLastLines[i]));
                    displayY += lineHeight;
                }
            }

            /* Line edit */
            RECT lineEditRect;
            lineEditRect.left = rect.left;
            lineEditRect.right = rect.right;
            lineEditRect.top = rect.bottom - lineHeight;
            lineEditRect.bottom = rect.bottom;
            // Draw line edit background
            HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &lineEditRect, blackBrush);
            DeleteObject(blackBrush);

            // Draw line edit text
            TextOutW(hdc, 5, rect.bottom - lineHeight, sLineEditBuffer, sLineEditLength);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN:
        {
            // Update keyboard state - mark key as pressed
            sKeyboardState[wParam] = 0x80;

            // Also update generic modifier keys when specific ones are pressed
            if (wParam == VK_LSHIFT || wParam == VK_RSHIFT)
                sKeyboardState[VK_SHIFT] = 0x80;
            if (wParam == VK_LCONTROL || wParam == VK_RCONTROL)
                sKeyboardState[VK_CONTROL] = 0x80;
            if (wParam == VK_LMENU || wParam == VK_RMENU)
                sKeyboardState[VK_MENU] = 0x80;

            // Handle Caps Lock toggle
            if (wParam == VK_CAPITAL)
            {
                sKeyboardState[VK_CAPITAL] ^= 0x01; // Toggle the low bit
            }

            // Don't process modifier keys as regular keys
            if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT ||
                wParam == VK_CONTROL || wParam == VK_LCONTROL || wParam == VK_RCONTROL ||
                wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU ||
                wParam == VK_CAPITAL)
            {
                return 0; // Just update state, don't process further
            }

            if (wParam == VK_RETURN)
            {
                Console_Run();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (wParam == VK_BACK)
            {
                if (sLineEditLength > 0)
                {
                    sLineEditLength--;
                    sLineEditBuffer[sLineEditLength] = '\0';
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            else
            {
                // Translate key to character using our maintained keyboard state
                WCHAR result[4];
                UINT scanCode = MapVirtualKey((UINT)wParam, MAPVK_VK_TO_VSC);
                int count = ToUnicode((UINT)wParam, scanCode, sKeyboardState, result, 4, 0);

                if (count > 0)
                {
                    // Add translated characters to input buffer
                    for (int i = 0; i < count; i++)
                    {
                        if (result[i] >= 32 && sLineEditLength < LINE_EDIT_BUFFER_SIZE - 1)
                        {
                            sLineEditBuffer[sLineEditLength++] = result[i];
                            sLineEditBuffer[sLineEditLength] = '\0';
                        }
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        }
        case WM_KEYUP:
        {
            // Update keyboard state - mark key as released
            sKeyboardState[wParam] = 0x00;

            // Also clear generic modifier keys when specific ones are released
            // But only if BOTH left and right are released
            if (wParam == VK_LSHIFT || wParam == VK_RSHIFT)
            {
                if (sKeyboardState[VK_LSHIFT] == 0 && sKeyboardState[VK_RSHIFT] == 0)
                    sKeyboardState[VK_SHIFT] = 0x00;
            }
            if (wParam == VK_LCONTROL || wParam == VK_RCONTROL)
            {
                if (sKeyboardState[VK_LCONTROL] == 0 && sKeyboardState[VK_RCONTROL] == 0)
                    sKeyboardState[VK_CONTROL] = 0x00;
            }
            if (wParam == VK_LMENU || wParam == VK_RMENU)
            {
                if (sKeyboardState[VK_LMENU] == 0 && sKeyboardState[VK_RMENU] == 0)
                    sKeyboardState[VK_MENU] = 0x00;
            }
            return 0;
        }
        case WM_CHAR:
        {
            // Don't handle WM_CHAR anymore - we're doing it in WM_KEYDOWN
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

        // Attach console input to game's thread for proper keyboard state
        DWORD gameThreadId = GetWindowThreadProcessId(gGameWindow, NULL);
        DWORD consoleThreadId = GetWindowThreadProcessId(sConsoleWindow, NULL);
        if (gameThreadId != consoleThreadId)
        {
            AttachThreadInput(consoleThreadId, gameThreadId, TRUE);
        }

        SetFocus(sConsoleWindow); // Steal focus from game
        UpdateWindow(sConsoleWindow);
    }
}

static void Console_Close(void)
{
    sLineEditBuffer[0] = '\0';
    sLineEditLength = 0;

    if (sConsoleWindow)
    {
        // Detach input before destroying
        DWORD gameThreadId = GetWindowThreadProcessId(gGameWindow, NULL);
        DWORD consoleThreadId = GetWindowThreadProcessId(sConsoleWindow, NULL);
        if (gameThreadId != consoleThreadId)
        {
            AttachThreadInput(consoleThreadId, gameThreadId, FALSE);
        }

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
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN || wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
            {
                SendMessage(sConsoleWindow, (UINT)wParam, p->vkCode, 0);
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

    Console_WriteLineWide(L"Console initialized");
    Console_WriteLineWide(L"This is a test!");
}
