#include <windows.h>
#include <stdint.h>

static void* sBaseAddr;

void* ResolveModulePtr(uint64_t ptr)
{
    /* Extract the base addr */
    if (!sBaseAddr)
    {
        HMODULE hModule = GetModuleHandleA(NULL);
        sBaseAddr = (void*)hModule;
    }

    return (char*)sBaseAddr + (ptr - 0x140000000);
}

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
    MessageBoxA(NULL, "CreateWindowExA called!", "Chantage Hook", MB_OK);
    return NULL;
}

BOOL CreateProcessA_HOOK(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
)
{
    MessageBoxA(NULL, "CreateProcessA called!", "Chantage Hook", MB_OK);
    return FALSE;
}

static void setupHooks(void)
{
    DWORD oldProtect;
    void** pCreateWindowExA;
    void** pCreateProcessA;

    pCreateWindowExA = (void**)ResolveModulePtr(0x14060c840);
    VirtualProtect(pCreateWindowExA, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    *pCreateWindowExA = (void*)&CreateWindowExA_HOOK;
    VirtualProtect(pCreateWindowExA, sizeof(void*), oldProtect, &oldProtect);

    pCreateProcessA = (void**)ResolveModulePtr(0x14060c390);
    VirtualProtect(pCreateProcessA, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    *pCreateProcessA = (void*)&CreateProcessA_HOOK;
    VirtualProtect(pCreateProcessA, sizeof(void*), oldProtect, &oldProtect);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    setupHooks();
    return TRUE;
}
