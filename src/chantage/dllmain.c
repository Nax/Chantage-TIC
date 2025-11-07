#include <windows.h>
#include <stdint.h>
#include <chantage/chantage.h>

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
    ChantageInit();
    return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
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
