#include <windows.h>
#include <chantage/chantage.h>

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

void HookFunction(void* target, void* hook)
{
    uint8_t buf[14] = {0xff, 0x25, 0x00, 0x00, 0x00, 0x00};
    memcpy(buf + 6, &hook, sizeof(void*));

    WriteProtected(target, buf, sizeof(buf));
}

void HookFunctionRel(uint32_t off, void* hook)
{
    HookFunction(BaseRelPtr(off), hook);
}

static uint64_t roundPageDown(uint64_t addr)
{
    return addr & ~0xfffULL;
}

static uint64_t roundPageUp(uint64_t addr)
{
    return (addr + 0xfff) & ~0xfffULL;
}

static void* Hook_AllocNearExecutable(void)
{
    uint64_t addr;
    uint64_t addrMin;
    uint64_t addrMax;

    /* Huge margin to not care about small addressing offsets */
    addr = (uint64_t)BaseRelPtr(0);
    addrMin = roundPageUp(addr - 0x70000000);
    addrMax = roundPageDown(addr + 0x70000000);

    /* Allocate memory in the range */
    void* result = NULL;
    for (uint64_t currentAddr = addrMin; currentAddr < addrMax; currentAddr += 0x1000)
    {
        result = VirtualAlloc((void*)currentAddr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (result)
            break;
    }

    return result;
}

void* Hook_CreateTrampoline(void* target)
{
    void* tramp;
    DWORD unusedOld;

    tramp = Hook_AllocNearExecutable();
    if (!tramp)
        return NULL;

    HookFunction(tramp, target);
    VirtualProtect(tramp, 14, PAGE_EXECUTE_READ, &unusedOld);

    return tramp;
}

void Hook_Call32(void* target, void* hook)
{
    uint64_t delta;

    delta = (uint64_t)hook - ((uint64_t)target + 5);
    target = (char*)target + 1;
    WriteProtected32(target, (uint32_t)delta);
}

void Hook_Call32Rel(uint32_t off, void* hook)
{
    void* target = BaseRelPtr(off);
    Hook_Call32(target, hook);
}

void Hook_InjectCall32(void* target, void* hook, int size)
{
    uint8_t buffer[16] = {0xe8, 0, 0, 0, 0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    uint64_t delta;
    delta = (uint64_t)hook - ((uint64_t)target + 5);
    memcpy(buffer + 1, &delta, sizeof(uint32_t));
    WriteProtected(target, buffer, size);
}

void Hook_InjectCall32Rel(uint32_t off, void* hook, int size)
{
    void* target = BaseRelPtr(off);
    Hook_InjectCall32(target, hook, size);
}

void Hook_CallTrampoline32(void* target, void* hook)
{
    void* tramp;

    tramp = Hook_CreateTrampoline(hook);
    if (!tramp)
        return;

    Hook_Call32(target, tramp);
}

void Hook_CallTrampoline32Rel(uint32_t off, void* hook)
{
    void* target = BaseRelPtr(off);
    Hook_CallTrampoline32(target, hook);
}
