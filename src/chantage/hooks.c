#include <windows.h>
#include <stdint.h>

static void* sBaseAddr;

void* BaseRelPtr(uint32_t off)
{
    /* Extract the base addr */
    if (!sBaseAddr)
    {
        HMODULE hModule = GetModuleHandleA(NULL);
        sBaseAddr = (void*)hModule;
    }

    return (char*)sBaseAddr + off;
}

void WriteProtected(void* dst, const void* src, size_t size)
{
    DWORD oldProtect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
}

void WriteProtected64(void* dst, uint64_t val)
{
    WriteProtected(dst, &val, sizeof(uint64_t));
}

void WriteProtected32(void* dst, uint32_t val)
{
    WriteProtected(dst, &val, sizeof(uint32_t));
}

void WriteProtected16(void* dst, uint16_t val)
{
    WriteProtected(dst, &val, sizeof(uint16_t));
}

void WriteProtected8(void* dst, uint8_t val)
{
    WriteProtected(dst, &val, sizeof(uint8_t));
}

void WriteProtectedRel(uint32_t off, const void* src, size_t size)
{
    void* dst = BaseRelPtr(off);
    WriteProtected(dst, src, size);
}

void WriteProtectedRel64(uint32_t off, uint64_t val)
{
    void* dst = BaseRelPtr(off);
    WriteProtected64(dst, val);
}

void WriteProtectedRel32(uint32_t off, uint32_t val)
{
    void* dst = BaseRelPtr(off);
    WriteProtected32(dst, val);
}

void WriteProtectedRel16(uint32_t off, uint16_t val)
{
    void* dst = BaseRelPtr(off);
    WriteProtected16(dst, val);
}

void WriteProtectedRel8(uint32_t off, uint8_t val)
{
    void* dst = BaseRelPtr(off);
    WriteProtected8(dst, val);
}
