#define _CRT_SECURE_NO_WARNINGS  1
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

static void injectDll(HANDLE hProcess)
{
    static const char* kDllName = "Chantage.dll";
    LPVOID alloc;
    LPVOID pLoadLibraryA;
    HANDLE thread;

    alloc = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProcess, alloc, kDllName, strlen(kDllName) + 1, NULL);
    pLoadLibraryA = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    thread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, alloc, 0, NULL);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    VirtualFreeEx(hProcess, alloc, 0, MEM_RELEASE);
}

static void launchInitial(void)
{
    PROCESS_INFORMATION proc;
    STARTUPINFOA si;

    /* Start the launcher */
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcessA("FFT_enhanced.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &proc))
    {
        printf("Failed to launch initial process (%lu)\n", GetLastError());
        ExitProcess(1);
    }
    printf("Launched process: %lu\n", proc.dwProcessId);
    ResumeThread(proc.hThread);
    WaitForSingleObject(proc.hProcess, INFINITE);
}

static HANDLE findGame(void)
{
    HANDLE snap;
    PROCESSENTRY32 pe;
    DWORD pid = 0;

    /* Wait for the game process to appear */
    while (pid == 0)
    {
        snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE)
        {
            Sleep(100);
            continue;
        }

        ZeroMemory(&pe, sizeof(pe));
        pe.dwSize = sizeof(pe);
        if (Process32First(snap, &pe))
        {
            do
            {
                if (_stricmp(pe.szExeFile, "FFT_enhanced.exe") == 0)
                {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(snap, &pe));
        }

        CloseHandle(snap);
        if (pid == 0)
            Sleep(100);
    }

    printf("Found game process: %lu\n", pid);
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

int main(void)
{
    HANDLE game;

    launchInitial();
    game = findGame();
    injectDll(game);

    return 0;
}
