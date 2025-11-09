#define _CRT_SECURE_NO_WARNINGS 1
#include <chantage/chantage.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>

#define ITEM_COUNT_VANILLA  0x105
#define ITEM_COUNT          0x200
#define ITEM_COUNT_EXTRA    (ITEM_COUNT - ITEM_COUNT_VANILLA)

static uint16_t sItemsCount = 0x105;
static ItemData sItems[ITEM_COUNT];
static ItemSubData sItemSubData[ITEM_COUNT];
static char* sItemDescriptionOverrides[ITEM_COUNT];
static char* sItemNameOverride[ITEM_COUNT];
static u8* gInventoryItemQuantity;

extern lua_State* gLuaState;
extern const char* kItemVanillaKeys[ITEM_COUNT_VANILLA];
const char* sItemExtraKeys[ITEM_COUNT_EXTRA];

typedef struct
{
    u32 flags;
    s32 nameRel;
    s32 nameSingularRel;
    s32 namePluralRel;
    s32 descriptionRel;
    s32 name2Rel;
    u8 unk1[0x04];
    u32 statusEffectId;
    u32 comment;
    u32 categoryId;
    u32 sortOrder;
    u8  unk2[1];
    u8  isRandomDamage;
    u8  pad[2];
}
ItemDatabaseEntry;

typedef struct
{
    ItemDatabaseEntry entry;
    char buffer[1024];
}
ItemDatabaseEntryResolved;

static ItemDatabaseEntryResolved sDatabasePool[4];
static int sDatabasePoolId = 0;

int Item_Count(void)
{
    return sItemsCount;
}

int Item_Lookup(const char* key)
{
    for (int i = 0; i < ITEM_COUNT_VANILLA; ++i)
    {
        if (strcmp(key, kItemVanillaKeys[i]) == 0)
            return i;
    }

    for (int i = 0; i < (sItemsCount - ITEM_COUNT_VANILLA); ++i)
    {
        if (sItemExtraKeys[i] && strcmp(key, sItemExtraKeys[i]) == 0)
            return ITEM_COUNT_VANILLA + i;
    }

    return -1;
}

const char* Item_ReverseLookup(u16 id)
{
    if (id < ITEM_COUNT_VANILLA)
        return kItemVanillaKeys[id];
    else if (id - ITEM_COUNT_VANILLA < (sItemsCount - ITEM_COUNT_VANILLA))
        return sItemExtraKeys[id - ITEM_COUNT_VANILLA];
    else
        return NULL;
}

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

    /* Item quantity stuff */
    WriteProtectedRel32(0x0ffe1b, sItemsCount - 1);
    WriteProtectedRel32(0x0fff09, sItemsCount - 1);
    WriteProtectedRel32(0x152ce1, sItemsCount);
    WriteProtectedRel32(0x2294f9, sItemsCount);
    WriteProtectedRel32(0x280346, sItemsCount); /* Warning: also checks another table, should be rerouted */
    WriteProtectedRel32(0x28079b, sItemsCount - 2);
    WriteProtectedRel32(0x2c1d6d, sItemsCount);
    WriteProtectedRel32(0x2d1fcf, sItemsCount);
    WriteProtectedRel32(0x303ba3, sItemsCount);
}

uint16_t Item_Alloc(const char* key)
{
    uint16_t id;
    char* keyBuf;

    id = sItemsCount++;
    keyBuf = Chantage_Alloc(strlen(key) + 1);
    strcpy(keyBuf, key);
    sItemExtraKeys[id - ITEM_COUNT_VANILLA] = keyBuf;
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

void Item_OverrideName(u16 itemId, const char* name)
{
    char* buf;

    if (!Item_IsValid(itemId))
        return;
    Chantage_Free(sItemNameOverride[itemId]);
    buf = Chantage_Alloc(strlen(name) + 1);
    strcpy(buf, name);
    sItemNameOverride[itemId] = buf;
}

void Item_OverrideDescription(u16 itemId, const char* description)
{
    char* buf;

    if (!Item_IsValid(itemId))
        return;
    Chantage_Free(sItemDescriptionOverrides[itemId]);
    buf = Chantage_Alloc(strlen(description) + 1);
    strcpy(buf, description);
    sItemDescriptionOverrides[itemId] = buf;
}

void AddWotlItems(void);

ItemDatabaseEntry* Item_GetDatabaseEntry(u16 itemId)
{
    ItemDatabaseEntry* (*sOriginalFunc)(u16 itemId);
    ItemDatabaseEntryResolved* pool;
    ItemDatabaseEntry* orig;
    const char* ptrName;
    const char* ptrDesc;
    const char* o;
    int cursorBuf;

    /* Resolve original function and original entry */
    sOriginalFunc = BaseRelPtr(0xf7510);
    orig = sOriginalFunc(itemId);

    /* Reserve an entry in the pool */
    pool = &sDatabasePool[sDatabasePoolId];
    sDatabasePoolId = (sDatabasePoolId + 1) % 4;
    memset(pool, 0, sizeof(*pool));

    /* Copy the original data (if any) */
    if (orig)
    {
        memcpy(&pool->entry, orig, sizeof(ItemDatabaseEntry));
        pool->entry.nameRel = 0;
        pool->entry.descriptionRel = 0;
        pool->entry.nameSingularRel = 0;
        pool->entry.namePluralRel = 0;
        pool->entry.name2Rel = 0;

        if (orig->nameRel)
            ptrName = (const char*)orig + orig->nameRel + 4;
        if (orig->descriptionRel)
            ptrDesc = (const char*)orig + orig->descriptionRel + 4;
    }

    if (itemId < sItemsCount)
    {
        o = sItemNameOverride[itemId];
        if (o)
            ptrName = o;
        o = sItemDescriptionOverrides[itemId];
        if (o)
            ptrDesc = o;
    }

    /* Copy the name in the buffer */
    if (ptrName)
    {
        pool->entry.nameRel = (pool->buffer + cursorBuf) - (char*)&pool->entry - 4;
        strcpy(pool->buffer + cursorBuf, ptrName);
        cursorBuf += strlen(ptrName) + 1;
    }

    if (ptrDesc)
    {
        pool->entry.descriptionRel = (pool->buffer + cursorBuf) - (char*)&pool->entry - 4;
        strcpy(pool->buffer + cursorBuf, ptrDesc);
        cursorBuf += strlen(ptrDesc) + 1;
    }

    /* Return the computed entry */
    return &pool->entry;
}

const char* Item_GetName(u16 itemId)
{
    ItemDatabaseEntry* entry;

    entry = Item_GetDatabaseEntry(itemId);
    if (!entry || !entry->nameRel)
        return NULL;
    return (const char*)entry + entry->nameRel + 4;
}

static void HookItemQuantity(void)
{
    gInventoryItemQuantity = Hook_AllocNearExecutable();

    Hook_InstrRef32Rel(0x030b2ea, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x00fff24, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x01f36af, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x02807af, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x00ffe96, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x0152ce7, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x0275d04, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x0275efe, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x02cb35f, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x0303bea, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x030b612, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x0205fbe, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x0207f22, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x02caa87, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x02d1ee1, gInventoryItemQuantity, 7);
    Hook_InstrRef32Rel(0x0391545, gInventoryItemQuantity, 7);

    Hook_InstrRefBase32Rel(0x0224e37, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x02f07f6, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x02c598c, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x02c1db4, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x027d7ba, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x027d6f6, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x027d819, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x022950c, gInventoryItemQuantity, 7);
    Hook_InstrRefBase32Rel(0x027d7d0, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x027d718, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x027d82f, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x0280353, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x022951a, gInventoryItemQuantity, 7);
    Hook_InstrRefBase32Rel(0x0222b8b, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x02e5e93, gInventoryItemQuantity, 8);
    Hook_InstrRefBase32Rel(0x0275063, gInventoryItemQuantity, 9);
}

void Item_SaveExtraData(const char* path)
{
    FILE* f;
    char buffer[_MAX_PATH];

    snprintf(buffer, _MAX_PATH, "%s\\ItemQty.bin", path);
    f = fopen(buffer, "wb");
    if (!f)
        return;
    fwrite(gInventoryItemQuantity, sizeof(u8), ITEM_COUNT, f);
    fclose(f);
}

void Item_LoadExtraData(const char* path)
{
    FILE* f;
    char buffer[_MAX_PATH];

    snprintf(buffer, _MAX_PATH, "%s\\ItemQty.bin", path);
    f = fopen(buffer, "rb");
    if (!f)
    {
        memset(gInventoryItemQuantity + 0x105, 0, ITEM_COUNT - 0x105);
        return;
    }
    fread(gInventoryItemQuantity, sizeof(u8), ITEM_COUNT, f);
    fclose(f);
}

int  Item_InventoyCount(u16 itemId)
{
    if (!Item_IsValid(itemId))
        return 0;
    return gInventoryItemQuantity[itemId];
}

void Item_InventoryAdd(u16 itemId, int quantity)
{
    int qty;

    if (!Item_IsValid(itemId))
        return;
    qty = gInventoryItemQuantity[itemId] + quantity;
    if (qty > 99)
        qty = 99;
    if (qty < 0)
        qty = 0;
    gInventoryItemQuantity[itemId] = (u8)qty;
}

void Item_InventorySet(u16 itemId, int quantity)
{
    if (!Item_IsValid(itemId))
        return;
    if (quantity > 99)
        quantity = 99;
    if (quantity < 0)
        quantity = 0;
    gInventoryItemQuantity[itemId] = (u8)quantity;
}

void ItemAPI_Register(void);

void Init_Items(void)
{
    void* Item_GetDatabaseEntryTrampoline;

    LoadItems();
    AddWotlItems();

    Item_GetDatabaseEntryTrampoline = Hook_CreateTrampoline(Item_GetDatabaseEntry);

    HookFunctionRel(0xe9f8a78, Item_IsValid);
    HookFunctionRel(0x02b4980, Item_GetData);
    HookFunctionRel(0xe978db8, Item_GetCategory);
    HookFunctionRel(0xe9983d0, Item_GetWeaponData);
    HookFunctionRel(0xe99fff0, Item_GetShieldData);
    HookFunctionRel(0xe9da940, Item_GetChemistData);
    HookFunctionRel(0xe9bae08, Item_GetArmorData);
    HookFunctionRel(0xe9d490e, Item_GetAccessoryData);
    HookFunctionRel(0x02b4668, Item_GetName);

    Hook_Call32Rel(0x1081dd, Item_GetDatabaseEntryTrampoline);
    Hook_Call32Rel(0x29b849, Item_GetDatabaseEntryTrampoline);
    Hook_Call32Rel(0x29cbcc, Item_GetDatabaseEntryTrampoline);
    Hook_Call32Rel(0x2c3c39, Item_GetDatabaseEntryTrampoline);
    Hook_Call32Rel(0x2f0690, Item_GetDatabaseEntryTrampoline);
    Hook_Call32Rel(0x2f069b, Item_GetDatabaseEntryTrampoline);

    HookItemQuantity();

    ItemAPI_Register();
}
