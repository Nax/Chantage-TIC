#include <windows.h>
#include <stdint.h>
#include <chantage/chantage.h>

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

static void setupHooks(void)
{
    DWORD oldProtect;
    void** pCreateWindowExA;
    void** pCreateProcessA;

    pCreateWindowExA = (void**)ResolveModulePtr(0x14060c840);
    //CreateWindowExA_Original = (void*)*pCreateWindowExA;
    VirtualProtect(pCreateWindowExA, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    *pCreateWindowExA = (void*)&CreateWindowExA_HOOK;
    VirtualProtect(pCreateWindowExA, sizeof(void*), oldProtect, &oldProtect);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    setupHooks();
    return TRUE;
}
