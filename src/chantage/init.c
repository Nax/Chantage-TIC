#include <windows.h>
#include <chantage/chantage.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

lua_State* gLuaState;

void Init_Game(void);
void Init_Console(void);

static void InitLua(void)
{
    gLuaState = luaL_newstate();
    luaL_openlibs(gLuaState);
}

static void ChantageInitImpl(void)
{
    InitLua();
    Init_Game();
    Init_Items();
    Init_Console();
}

void ChantageInit(void)
{
    static int sIsInitialized = 0;

    if (sIsInitialized)
        return;
    sIsInitialized = 1;
    ChantageInitImpl();
}
