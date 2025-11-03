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
