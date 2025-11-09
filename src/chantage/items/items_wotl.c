#include <chantage/chantage.h>

typedef struct
{
    const char* key;
    const char* name;
    const char* desc;
    u8 type;
    u8 flags;
}
ItemInputBase;

typedef struct
{
    ItemInputBase base;
    u8 hp;
    u8 mp;
}
ItemInputArmor;

static const char kTestDesc[] = "This is a test description for a WotL item.";

static const ItemInputArmor kWotlArmors[] = {
    { {"wotl:vanguard_helm", "Vanguard Helm", kTestDesc, ITEM_TYPE_HELMET, 0x22 }, 150, 20 },
    { {"wotl:onion_helm", "Onion Helm", kTestDesc, ITEM_TYPE_HELMET, 0x22 }, 200, 0 },
};

static u16 Wotl_AddItemInput(const ItemInputBase* input)
{
    ItemData* item;
    u16 itemId;

    itemId = Item_Alloc(input->key);
    item = Item_GetData(itemId);

    item->flags = input->flags;
    item->price = 10;
    item->type = input->type;
    item->shop = 20;

    Item_OverrideName(itemId, input->name);
    Item_OverrideDescription(itemId, input->desc);

    return itemId;
}

static u16 Wotl_AddArmor(const ItemInputArmor* armorInput)
{
    u16 itemId;
    ItemArmorData* armorData;

    itemId = Wotl_AddItemInput(&armorInput->base);
    armorData = Item_GetArmorData(itemId);
    armorData->hp = armorInput->hp;
    armorData->mp = armorInput->mp;

    return itemId;
}

/*
 * This is temporary, will be moved to a proper mod later on.
 */
void AddWotlItems(void)
{
    for (int i = 0; i < sizeof(kWotlArmors) / sizeof(kWotlArmors[0]); ++i)
        Wotl_AddArmor(&kWotlArmors[i]);
}
