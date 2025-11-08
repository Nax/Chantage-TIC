#include <chantage/chantage.h>
#include <stdlib.h>
#include <string.h>

#define ITEM_COUNT 0x200

static ItemData sItems[ITEM_COUNT];
static ItemSubData sItemSubData[ITEM_COUNT];
static char* sItemDescriptionsOverrides[ITEM_COUNT];
static uint16_t sItemsCount = 0x105;

int Item_IsValid(u16 id)
{
    return (id != 0xfe && id != 0xff && id < sItemsCount);
}

static ItemData* Item_GetData(uint16_t itemId)
{
    if (itemId >= sItemsCount)
        return NULL;
    return &sItems[itemId];
}

ItemWeaponData* Item_GetWeaponData(uint16_t itemId)
{
    int cat;

    cat = Item_GetCategory(itemId);
    if (cat != ITEM_CATEGORY_WEAPON && cat != ITEM_CATEGORY_THROW)
        return NULL;
    return &sItemSubData[itemId].weapon;
}

ItemShieldData* Item_GetShieldData(uint16_t itemId)
{
    if (Item_GetCategory(itemId) != ITEM_CATEGORY_SHIELD)
        return NULL;
    return &sItemSubData[itemId].shield;
}

ItemArmorData* Item_GetArmorData(uint16_t itemId)
{
    int cat;

    cat = Item_GetCategory(itemId);
    if (cat != ITEM_CATEGORY_HELM && cat != ITEM_CATEGORY_ARMOR)
        return NULL;
    return &sItemSubData[itemId].armor;
}

ItemAccessoryData* Item_GetAccessoryData(uint16_t itemId)
{
    if (Item_GetCategory(itemId) != ITEM_CATEGORY_ACCESSORY)
        return NULL;
    return &sItemSubData[itemId].accessory;
}

ItemChemistData* Item_GetChemistData(uint16_t itemId)
{
    if (Item_GetCategory(itemId) != ITEM_CATEGORY_CHEMIST)
        return NULL;
    return &sItemSubData[itemId].chemist;
}

int Item_GetCategory(uint16_t itemId)
{
    ItemData* item;

    if (itemId == 0xfe || itemId == 0xff)
        return ITEM_CATEGORY_NONE;
    item = Item_GetData(itemId);
    if (!item)
        return ITEM_CATEGORY_NONE;

    switch (item->type)
    {
    case ITEM_TYPE_NONE:
    case ITEM_TYPE_KNIFE:
    case ITEM_TYPE_NINJABLADE:
    case ITEM_TYPE_SWORD:
    case ITEM_TYPE_KNIGHTSWORD:
    case ITEM_TYPE_KATANA:
    case ITEM_TYPE_AXE:
    case ITEM_TYPE_ROD:
    case ITEM_TYPE_STAFF:
    case ITEM_TYPE_FLAIL:
    case ITEM_TYPE_GUN:
    case ITEM_TYPE_CROSSBOW:
    case ITEM_TYPE_BOW:
    case ITEM_TYPE_INSTRUMENT:
    case ITEM_TYPE_BOOK:
    case ITEM_TYPE_POLEARM:
    case ITEM_TYPE_POLE:
    case ITEM_TYPE_BAG:
    case ITEM_TYPE_CLOTH:
        return ITEM_CATEGORY_WEAPON;
    case ITEM_TYPE_THROWING:
    case ITEM_TYPE_BOMB:
        return ITEM_CATEGORY_THROW;
    case ITEM_TYPE_SHIELD:
        return ITEM_CATEGORY_SHIELD;
    case ITEM_TYPE_HELMET:
    case ITEM_TYPE_HAT:
    case ITEM_TYPE_HAIRADORN:
        return ITEM_CATEGORY_HELM;
    case ITEM_TYPE_ARMOR:
    case ITEM_TYPE_CLOTHING:
    case ITEM_TYPE_ROBE:
        return ITEM_CATEGORY_ARMOR;
    case ITEM_TYPE_SHOES:
    case ITEM_TYPE_ARMGUARD:
    case ITEM_TYPE_RING:
    case ITEM_TYPE_ARMLET:
    case ITEM_TYPE_CLOAK:
    case ITEM_TYPE_PERFUME:
        return ITEM_CATEGORY_ACCESSORY;
    case ITEM_TYPE_CHEMIST:
        return ITEM_CATEGORY_CHEMIST;
    default:
        return ITEM_CATEGORY_NONE;
    }
}

static void Item_PatchCount(void)
{
    uint8_t countLea;
    countLea = sItemsCount - 0xff;

    /* Beware, this is probably a signed offset */
    WriteProtectedRel8(0x280ba6, countLea);
    WriteProtectedRel32(0x27c2b3, sItemsCount);
    WriteProtectedRel32(0x281dc9, sItemsCount);
    WriteProtectedRel32(0x282293, sItemsCount);
    WriteProtectedRel32(0x284c76, sItemsCount);
    WriteProtectedRel32(0x285010, sItemsCount);
    WriteProtectedRel32(0x2a2824, sItemsCount);
    WriteProtectedRel32(0x2b9796, sItemsCount);
    WriteProtectedRel32(0x2c003c, sItemsCount);
    WriteProtectedRel32(0x2c3a90, sItemsCount);
    WriteProtectedRel32(0x2fd393, sItemsCount);
    WriteProtectedRel32(0x300bc6, sItemsCount);
    WriteProtectedRel32(0x318038, sItemsCount);
    WriteProtectedRel32(0x3181d9, sItemsCount);
    WriteProtectedRel32(0x32a4cd, sItemsCount);
    WriteProtectedRel32(0x32a5aa, sItemsCount);
    WriteProtectedRel32(0x32b116, sItemsCount);
    WriteProtectedRel32(0x32b295, sItemsCount);
    WriteProtectedRel32(0x335a36, sItemsCount);
    WriteProtectedRel32(0x335bd7, sItemsCount);
    WriteProtectedRel32(0x35b638, sItemsCount);
    WriteProtectedRel32(0x391b47, sItemsCount);
    WriteProtectedRel32(0x39519d, sItemsCount);
    WriteProtectedRel32(0x39527a, sItemsCount);
}

uint16_t Item_Alloc(void)
{
    uint16_t id;

    id = sItemsCount++;
    Item_PatchCount();

    return id;
}

static void LoadItems(void)
{
    ItemData* srcItems;

    /* Copy the existing items */
    srcItems = BaseRelPtr(0x808740);
    for (int i = 0; i < 0x100; ++i)
        memcpy(&sItems[i], &srcItems[i], sizeof(ItemData));
    srcItems = BaseRelPtr(0x67b470);
    for (int i = 0; i < 5; ++i)
        memcpy(&sItems[i + 0x100], &srcItems[i], sizeof(ItemData));
}

static void AddWotlItems(void)
{
    ItemData* item;
    ItemArmorData* armor;
    uint16_t itemId;

    /* Vanguard helm */
    itemId = Item_Alloc();
    item = Item_GetData(itemId);
    item->palette = 0x03;
    item->gfx = 0x55;
    item->flags = 0x22;
    item->price = 10;
    item->type = ITEM_TYPE_HELMET;
    item->shop = 0x14;
    armor = Item_GetArmorData(itemId);
    armor->hp = 150;
    armor->mp = 20;
    sItemDescriptionsOverrides[itemId] = "A test helmet!";
}

void* Item_GetDescriptionDisplay(u16 itemId)
{
    static u32 sBuffer[5];
    void* (*sOriginalFunc)(u16 itemId);
    char* override;
    void* ret;

    override = NULL;
    if (itemId < sItemsCount)
        override = sItemDescriptionsOverrides[itemId];

    if (override)
    {
        sBuffer[4] = (u64)(override - (char*)sBuffer - 4);
        ret = sBuffer;
    }
    else
    {
        sOriginalFunc = BaseRelPtr(0xf7510);
        ret = sOriginalFunc(itemId);
    }

    return ret;
}

void Init_Items(void)
{
    LoadItems();
    AddWotlItems();

    HookFunctionRel(0xe9f8a78, Item_IsValid);
    HookFunctionRel(0x02b4980, Item_GetData);
    HookFunctionRel(0xe978db8, Item_GetCategory);
    HookFunctionRel(0xe9983d0, Item_GetWeaponData);
    HookFunctionRel(0xe99fff0, Item_GetShieldData);
    HookFunctionRel(0xe9da940, Item_GetChemistData);
    HookFunctionRel(0xe9bae08, Item_GetArmorData);
    HookFunctionRel(0xe9d490e, Item_GetAccessoryData);
    Hook_CallTrampoline32Rel(0x29cbcc, Item_GetDescriptionDisplay);
    //HookFunctionRel(0x00f7510, Item_GetDescription);
}
