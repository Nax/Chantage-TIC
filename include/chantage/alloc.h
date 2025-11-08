#ifndef CHANTAGE_ALLOC_H
#define CHANTAGE_ALLOC_H

void*   Chantage_Alloc(size_t size);
void    Chantage_Free(void* ptr);
void*   Chantage_Realloc(void* ptr, size_t newSize);

#endif
