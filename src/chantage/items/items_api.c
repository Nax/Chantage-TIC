#include <chantage/chantage.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern lua_State* gLuaState;

static int ItemAPI_Lookup(lua_State* L, int idx)
{
    int id;
    const char* key;

    if (lua_type(L, idx) == LUA_TNUMBER)
    {
        id = (int)luaL_checkinteger(L, idx);
        if (id < 0 || id >= Item_Count())
            return -1;
        return id;
    }

    if (lua_type(L, idx) == LUA_TSTRING)
    {
        key = lua_tostring(L, idx);
        return Item_Lookup(key);
    }

    return -1;
}

static int ItemAPI_Func_ItemId(lua_State* L)
{
    int id;

    id = ItemAPI_Lookup(L, 1);
    if (id < 0)
        lua_pushnil(L);
    else
        lua_pushinteger(L, id);
    return 1;
}

static int ItemAPI_Func_ItemKey(lua_State* L)
{
    int id;
    const char* key;

    key = NULL;
    id = ItemAPI_Lookup(L, 1);
    if (id >= 0)
        key = Item_ReverseLookup((u16)id);
    if (key)
        lua_pushstring(L, key);
    else
        lua_pushnil(L);
    return 1;
}

void ItemAPI_Register(void)
{
    lua_newtable(gLuaState);

    lua_pushcfunction(gLuaState, ItemAPI_Func_ItemId);
    lua_setfield(gLuaState, -2, "id");

    lua_pushcfunction(gLuaState, ItemAPI_Func_ItemKey);
    lua_setfield(gLuaState, -2, "key");

    lua_setglobal(gLuaState, "Item");
}
