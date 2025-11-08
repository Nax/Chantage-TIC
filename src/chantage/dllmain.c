#include <windows.h>
#include <stdint.h>
#include <chantage/chantage.h>

HWND gGameWindow;

void ChantageInit(void);

static HWND (*CreateWindowExA_Original)(
  DWORD     dwExStyle,
  LPCSTR    lpClassName,
  LPCSTR    lpWindowName,
  DWORD     dwStyle,
  int       X,
  int       Y,
  int       nWidth,
  int       nHeight,
  HWND      hWndParent,
  HMENU     hMenu,
  HINSTANCE hInstance,
  LPVOID    lpParam
);

HWND CreateWindowExA_HOOK(
  DWORD     dwExStyle,
  LPCSTR    lpClassName,
  LPCSTR    lpWindowName,
  DWORD     dwStyle,
  int       X,
  int       Y,
  int       nWidth,
  int       nHeight,
  HWND      hWndParent,
  HMENU     hMenu,
  HINSTANCE hInstance,
  LPVOID    lpParam
)
{
    HWND window;

    window = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    if (strcmp(lpClassName, "SplashClass") != 0)
        gGameWindow = window;
    ChantageInit();
    return window;
}

BOOL IsDebuggerPresent_HOOK(void)
{
    return FALSE;
}

static void setupHooks(void)
{
    WriteProtectedRel64(0x60c840, (uint64_t)&CreateWindowExA_HOOK);
    WriteProtectedRel64(0x60c320, (uint64_t)&IsDebuggerPresent_HOOK);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    setupHooks();
    return TRUE;
}
