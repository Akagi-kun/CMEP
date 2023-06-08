#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma comment(lib, "lua54.lib")

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}