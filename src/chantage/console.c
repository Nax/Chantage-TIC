#include <windows.h>
#include <chantage/chantage.h>

#define MAX_LINES 200
#define LINE_EDIT_BUFFER_SIZE 1024

extern HWND gGameWindow;
static ATOM sConsoleClass;
static HWND sConsoleWindow;

static PWCH sLastLines[MAX_LINES];
static WCHAR sLineEditBuffer[LINE_EDIT_BUFFER_SIZE];
static int sLineEditLength = 0;

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
    /* TODO */

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
                // Translate key to character using current keyboard state
                BYTE keyState[256];
                GetKeyboardState(keyState);
                WCHAR result[4];
                int count = ToUnicode((UINT)wParam, MapVirtualKey((UINT)wParam, MAPVK_VK_TO_VSC), keyState, result, 4, 0);
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
