#include <windows.h>
#include "minhook/MinHook.h"

using namespace std;

BOOL CALLBACK enumFunc(HWND hwnd, LPARAM lParam)
{
    DWORD thisId;
    GetWindowThreadProcessId(hwnd, &thisId);
    if (thisId == GetCurrentProcessId()) {
        *(HWND*)lParam = hwnd;
        return FALSE; // stop enumeration
    }

    // Continue enumerating
    return TRUE;
}

HWND findTopWindow()
{
    HWND hwnd = 0;
    EnumWindows(enumFunc, (LPARAM)&hwnd);
    return hwnd;
}

bool scramble = false;

DWORD WINAPI annoyThread(LPVOID lpParameter)
{
    HWND mainWnd = findTopWindow();
    while(true) {
        scramble = true;
        MessageBoxA(mainWnd, "Hello!!!", "Caption", MB_OK);
        scramble = false;
        Sleep(5000);
    }
}


decltype(GetSysColor)* originalGetSysColor = NULL;
DWORD WINAPI myGetSysColor(int nIndex) {
    return RGB(255,0,0);
}


void scrambleText(wchar_t* text, int count) {
    if (scramble && text != NULL) {
        for (int i = 0; i < count; ++i) {
            auto c = text[i];
            if (c >= 'a' && c <= 'z') {
                c = (((c - 'a') + 13) % ('z' - 'a')) + 'A';
                text[i] = c;
            }
        }
    }
}


decltype(DrawTextExW)* originalDrawTextExW = NULL;
int WINAPI myDrawTextExW(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp)
{
    scrambleText(lpchText, cchText);
    return originalDrawTextExW(hdc, lpchText, cchText, lprc, format, lpdtp);
}

decltype(ExtTextOutW)* originalExtTextOutW = NULL;
BOOL WINAPI myExtTextOutW(HDC hdc, int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
    scrambleText((LPWSTR)lpString, c);
    return originalExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
}


void doHook()
{


    if (MH_Initialize() != MH_OK)
        return;
    if (MH_CreateHook(&GetSysColor, &myGetSysColor, (void**)(&originalGetSysColor)) != MH_OK)
        return;
    if (MH_EnableHook(&GetSysColor) != MH_OK)
        return;

    if (MH_CreateHook(&DrawTextExW, &myDrawTextExW, (void**)(&originalDrawTextExW)) != MH_OK)
        return;
    if (MH_EnableHook(&DrawTextExW) != MH_OK)
        return;

    if (MH_CreateHook(&ExtTextOutW, &myExtTextOutW, (void**)(&originalExtTextOutW)) != MH_OK)
        return;
    if (MH_EnableHook(&ExtTextOutW) != MH_OK)
        return;

}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        doHook();
        CreateThread(NULL, 0, annoyThread, 0, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

