#include <chantage/chantage.h>

static const ItemData sItemData = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

const ItemData* Item_GetData(uint16_t itemId)
{
    return &sItemData;
}

void Init_Items(void)
{
    HookFunctionRel(0x2b4980, (void*)Item_GetData);
}
