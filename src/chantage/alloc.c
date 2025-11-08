#include <windows.h>
#include <chantage/alloc.h>

void* Chantage_Alloc(size_t size)
{
    if (size == 0)
        return NULL;
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void Chantage_Free(void* ptr)
{
    if (ptr)
        HeapFree(GetProcessHeap(), 0, ptr);
}

void* Chantage_Realloc(void* ptr, size_t newSize)
{
    if (!ptr)
        return Chantage_Alloc(newSize);
    if (newSize == 0)
    {
        Chantage_Free(ptr);
        return NULL;
    }
    return HeapReAlloc(GetProcessHeap(), 0, ptr, newSize);
}
