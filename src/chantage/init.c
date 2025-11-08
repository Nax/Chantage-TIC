#include <windows.h>
#include <chantage/chantage.h>

void Init_Game(void);

static void ChantageInitImpl(void)
{
    Init_Game();
    Init_Items();
}

void ChantageInit(void)
{
    static int sIsInitialized = 0;

    if (sIsInitialized)
        return;
    sIsInitialized = 1;
    ChantageInitImpl();
}
