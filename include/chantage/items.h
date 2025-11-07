#ifndef CHANTAGE_ITEMS_H
#define CHANTAGE_ITEMS_H

#include <chantage/types.h>

#define ITEM_CATEGORY_NONE          (-1)
#define ITEM_CATEGORY_WEAPON        0
#define ITEM_CATEGORY_THROW         1
#define ITEM_CATEGORY_SHIELD        2
#define ITEM_CATEGORY_HELM          3
#define ITEM_CATEGORY_ARMOR         4
#define ITEM_CATEGORY_ACCESSORY     5
#define ITEM_CATEGORY_CHEMIST       6

#define ITEM_TYPE_NONE          0x00
#define ITEM_TYPE_KNIFE         0x01
#define ITEM_TYPE_NINJABLADE    0x02
#define ITEM_TYPE_SWORD         0x03
#define ITEM_TYPE_KNIGHTSWORD   0x04
#define ITEM_TYPE_KATANA        0x05
#define ITEM_TYPE_AXE           0x06
#define ITEM_TYPE_ROD           0x07
#define ITEM_TYPE_STAFF         0x08
#define ITEM_TYPE_FLAIL         0x09
#define ITEM_TYPE_GUN           0x0a
#define ITEM_TYPE_CROSSBOW      0x0b
#define ITEM_TYPE_BOW           0x0c
#define ITEM_TYPE_INSTRUMENT    0x0d
#define ITEM_TYPE_BOOK          0x0e
#define ITEM_TYPE_POLEARM       0x0f
#define ITEM_TYPE_POLE          0x10
#define ITEM_TYPE_BAG           0x11
#define ITEM_TYPE_CLOTH         0x12
#define ITEM_TYPE_SHIELD        0x13
#define ITEM_TYPE_HELMET        0x14
#define ITEM_TYPE_HAT           0x15
#define ITEM_TYPE_HAIRADORN     0x16
#define ITEM_TYPE_ARMOR         0x17
#define ITEM_TYPE_CLOTHING      0x18
#define ITEM_TYPE_ROBE          0x19
#define ITEM_TYPE_SHOES         0x1a
#define ITEM_TYPE_ARMGUARD      0x1b
#define ITEM_TYPE_RING          0x1c
#define ITEM_TYPE_ARMLET        0x1d
#define ITEM_TYPE_CLOAK         0x1e
#define ITEM_TYPE_PERFUME       0x1f
#define ITEM_TYPE_THROWING      0x20
#define ITEM_TYPE_BOMB          0x21
#define ITEM_TYPE_CHEMIST       0x22

typedef struct
{
    u8  palette;
    u8  gfx;
    u8  level;
    u8  flags;
    u8  unk0;
    u8  type;
    u8  unk1;
    u8  attrId;
    u16 price;
    u8  shop;
    u8  unk2;
}
ItemData;

_Static_assert(sizeof(ItemData) == 0x0c, "ItemData size incorrect");

typedef struct
{
    u8 range;
    u8 flags;
    u8 formula;
    u8 unk;
    u8 wp;
    u8 evade;
    u8 elements;
    u8 status;
}
ItemWeaponData;

typedef struct
{
    u8 blockPhysical;
    u8 blockMagical;
}
ItemShieldData;

typedef struct
{
    u8 hp;
    u8 mp;
}
ItemArmorData;

typedef struct
{
    u8 evadePhysical;
    u8 evadeMagical;
}
ItemAccessoryData;

typedef struct
{
    u8 formula;
    u8 z;
    u8 status;
}
ItemChemistData;

typedef union
{
    ItemWeaponData     weapon;
    ItemShieldData     shield;
    ItemArmorData      armor;
    ItemAccessoryData  accessory;
    ItemChemistData    chemist;
}
ItemSubData;

void Init_Items(void);

ItemData*           Item_GetData(uint16_t itemId);
int                 Item_GetCategory(uint16_t itemId);
ItemWeaponData*     Item_GetWeaponData(uint16_t itemId);
ItemShieldData*     Item_GetShieldData(uint16_t itemId);
ItemArmorData*      Item_GetArmorData(uint16_t itemId);
ItemAccessoryData*  Item_GetAccessoryData(uint16_t itemId);
ItemChemistData*    Item_GetChemistData(uint16_t itemId);

#endif
