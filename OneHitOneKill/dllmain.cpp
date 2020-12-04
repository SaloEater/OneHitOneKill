// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "patcher_x86.h"
#include <windows.h>

Patcher* patcher = GetPatcher();
PatcherInstance* patcherInstance = patcher->CreateInstance((char*)"Own Proj");

int __stdcall LoHook_combatMonster_DoPhysicalDamage_Oneshot(LoHook* hook, HookContext* context)
{
    MessageBox(0, L""+*(__int32*)(context->ebp + 8), L"OK", MB_OK);
   *(__int32*)(context->ebp + 8) = 32167;
   context->return_address = 0x443DBB;
   return NO_EXEC_DEFAULT;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        patcherInstance->WriteLoHook(0x443DB8, LoHook_combatMonster_DoPhysicalDamage_Oneshot);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

