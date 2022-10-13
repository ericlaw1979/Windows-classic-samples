#include "Globals.h"
#include "PopupWindow.h"

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hInst = hInstance;

            if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0))
                return FALSE;

            CPropertyPopupWindow::StaticInit();

            break;

        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&g_cs);
            break;
    }

    return TRUE;
}
