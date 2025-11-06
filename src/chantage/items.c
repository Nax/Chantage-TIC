#include <chantage/chantage.h>
#include <stdlib.h>
#include <string.h>

int sExtraItemCount;
int sExtraItemCapacity;
ItemData* sExtraItems;

ItemData* Item_GetData(uint16_t itemId)
{
    ItemData* table;

    if (itemId >= 0x140)
    {
        itemId -= 0x140;
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

static void Item_PatchCount(void)
{
    uint16_t count;
    uint8_t countLea;

    count = 0x140 + sExtraItemCount;
    countLea = count - 0xff;

    /* Beware, this is probably a signed offset */
    WriteProtectedRel8(0x280ba6, countLea);
}

uint16_t Item_Alloc(void)
{
    uint16_t id;

    if (sExtraItemCount >= sExtraItemCapacity)
    {
        sExtraItemCapacity *= 2;
        sExtraItems = realloc(sExtraItems, sizeof(ItemData) * sExtraItemCapacity);
    }
    id = 0x140 + sExtraItemCount;
    memset(&sExtraItems[sExtraItemCount], 0, sizeof(ItemData));
    sExtraItemCount++;
    Item_PatchCount();
    return id;
}

/* Should be changed into a mod eventually*/
static void AddWotlItems(void)
{
    ItemData* item;
    uint16_t itemId;

    /* Vanguard helm */
    itemId = Item_Alloc();
    item = Item_GetData(itemId);
    item->palette = 3;
    item->gfx = 0x55;
    item->flags = 0x22;
    item->price = 10;
    item->type = 0x14;
    item->shop = 0x14;
}

void Init_Items(void)
{
    sExtraItemCount = 0;
    sExtraItemCapacity = 8;
    sExtraItems = malloc(sizeof(ItemData) * sExtraItemCapacity);

    HookFunctionRel(0x02b4980, (void*)Item_GetData);
    HookFunctionRel(0xe978db8, (void*)Item_GetCategory);
}
