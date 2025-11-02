#define _CRT_SECURE_NO_WARNINGS  1
#include <windows.h>

int main(void)
{
    PROCESS_INFORMATION proc;
    STARTUPINFOA si;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcessA("FFT_enhanced.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &proc))
    {
        wchar_t buf[256];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
        MessageBoxW(NULL, buf, NULL, 0);
        return 1;
    }

    ResumeThread(proc.hThread);
    return 0;
}
