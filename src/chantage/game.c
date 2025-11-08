#include <windows.h>
#include <shlobj.h>
#include <chantage/chantage.h>
#include <stdio.h>

static const char* Game_GetSavedGamesPath(void)
{
    static char savePath[MAX_PATH];
    static int initialized = 0;

    if (!initialized)
    {
        PWSTR savedGamesPath = NULL;
        HRESULT hr = SHGetKnownFolderPath(&FOLDERID_SavedGames, 0, NULL, &savedGamesPath);

        if (SUCCEEDED(hr))
        {
            // Convert wide string to multibyte string
            WideCharToMultiByte(CP_UTF8, 0, savedGamesPath, -1, savePath, MAX_PATH, NULL, NULL);
            CoTaskMemFree(savedGamesPath);
            initialized = 1;
            MessageBoxA(NULL, savePath, "Saved Games Path", MB_OK);
        }
        else
        {
            // Fallback to empty string if SavedGames is not available
            savePath[0] = '\0';
            MessageBoxA(NULL, "Failed to get Saved Games Path", "Error", MB_OK);
        }
    }

    return savePath;
}

static const char* Game_CustomSaveRoot(void)
{
    static char customPath[MAX_PATH];
    static int initialized = 0;

    if (!initialized)
    {
        const char* basePath = Game_GetSavedGamesPath();
        if (strlen(basePath) > 0)
        {
            snprintf(customPath, MAX_PATH, "%s\\TheIvaliceChroniclesModsData", basePath);
            CreateDirectoryA(customPath, NULL);
            MessageBoxA(NULL, customPath, "Custom Save Root", MB_OK);
        }
        else
        {
            customPath[0] = '\0';
        }
        initialized = 1;
    }

    return customPath;
}

static const char* Game_CustomSavePath(const char* suffix)
{
    static char savePath[MAX_PATH];

    const char* rootPath = Game_CustomSaveRoot();
    if (strlen(rootPath) > 0)
    {
        snprintf(savePath, MAX_PATH, "%s\\%s", rootPath, suffix);
        CreateDirectoryA(savePath, NULL);
    }
    else
    {
        savePath[0] = '\0';
    }

    return savePath;
}

#if 0
void Game_SaveLoadWrapper(void* saveBuf, int isLoad)
{
    void (*Game_SaveLoad)(void*, int);

    Game_SaveLoad = BaseRelPtr(0x275c58);
    Game_SaveLoad(saveBuf, isLoad);

    if (isLoad)
        Game_LoadExtraData();
    else
        Game_SaveExtraData();
}
#endif


#if 0
void Init_Game(void)
{
    void* Game_SaveLoadTrampoline;

    Game_SaveLoadTrampoline = Hook_CreateTrampoline(Game_SaveLoadWrapper);
    Hook_Call32Rel(0x02c999a, Game_SaveLoadTrampoline);
    Hook_Call32Rel(0x02c9ee0, Game_SaveLoadTrampoline);
}
#endif

void Game_SaveExtraData(void)
{
    MessageBoxA(NULL, "Saving Extra Data", "Info", MB_OK);
    const char* path = Game_CustomSavePath("Save00");
    Item_SaveExtraData(path);
}

void Game_LoadExtraData(void)
{
    MessageBoxA(NULL, "Loading Extra Data", "Info", MB_OK);
    const char* path = Game_CustomSavePath("Save00");
    Item_LoadExtraData(path);
}

void Game_SaveReadWrapper(int unk)
{
    /* Handle normal load */
    void (*Game_SaveRead)(int);
    Game_SaveRead = BaseRelPtr(0x207e3c);
    Game_SaveRead(unk);

    Game_LoadExtraData();
}

void Game_SaveWriteWrapper(int unk)
{
    /* Handle normal save */
    void (*Game_SaveWrite)(int);
    Game_SaveWrite = BaseRelPtr(0x207dc4);
    Game_SaveWrite(unk);

    Game_SaveExtraData();
}

void Init_Game(void)
{
    void* Game_SaveReadTrampoline;
    void* Game_SaveWriteTrampoline;

    Game_SaveReadTrampoline = Hook_CreateTrampoline(Game_SaveReadWrapper);
    Hook_Call32Rel(0x00c3ca3, Game_SaveReadTrampoline);
    Hook_Call32Rel(0x0207896, Game_SaveReadTrampoline);
    Hook_Call32Rel(0x02b7888, Game_SaveReadTrampoline);
    Hook_Call32Rel(0x02cbc88, Game_SaveReadTrampoline);
    Hook_Call32Rel(0x0321aec, Game_SaveReadTrampoline);
    Hook_Call32Rel(0x032237b, Game_SaveReadTrampoline);

    Game_SaveWriteTrampoline = Hook_CreateTrampoline(Game_SaveWriteWrapper);
    Hook_Call32Rel(0x00c3b23, Game_SaveWriteTrampoline);
    Hook_Call32Rel(0x0207c58, Game_SaveWriteTrampoline);
    Hook_Call32Rel(0x0231f39, Game_SaveWriteTrampoline);
    Hook_Call32Rel(0x02b7819, Game_SaveWriteTrampoline);
    Hook_Call32Rel(0x02ca794, Game_SaveWriteTrampoline);
}
