#include <windows.h>
#include <chantage/chantage.h>

static void ChantageInitImpl(void)
{
    MessageBoxA(NULL, "Chantage initialized!", "Chantage", MB_OK);
}

void ChantageInit(void)
{
    static int sIsInitialized = 0;

    if (sIsInitialized)
        return;
    sIsInitialized = 1;
    ChantageInitImpl();
}
