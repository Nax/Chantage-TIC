#include <chantage/chantage.h>
#include <stdlib.h>

int sExtraItemCount;
int sExtraItemCapacity;
ItemData* sExtraItems;

ItemData* Item_GetData(uint16_t itemId)
{
    ItemData* table;

    if (itemId >= 0x200)
    {
        itemId -= 0x200;
        table = sExtraItems;
    }
    else if (itemId >= 0x100)
    {
        itemId -= 0x100;
        table = BaseRelPtr(0x67a870);
    }
    else
    {
        table = BaseRelPtr(0x808740);
    }

    return table + itemId;
}

uint16_t Item_Alloc(void)
{
    uint16_t id;

    if (sExtraItemCount >= sExtraItemCapacity)
    {
        sExtraItemCapacity *= 2;
        sExtraItems = realloc(sExtraItems, sizeof(ItemData) * sExtraItemCapacity);
    }
    id = 0x200 + sExtraItemCount;
    sExtraItemCount++;
    return id;
}

void Init_Items(void)
{
    sExtraItemCount = 0;
    sExtraItemCapacity = 8;
    sExtraItems = malloc(sizeof(ItemData) * sExtraItemCapacity);

    HookFunctionRel(0x2b4980, (void*)Item_GetData);
}
