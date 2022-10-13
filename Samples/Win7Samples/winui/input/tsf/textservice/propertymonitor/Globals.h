#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <assert.h>
#include <strsafe.h>
#include "msctf.h"

void DllAddRef();
void DllRelease();

// All languages
#define TEXTSERVICE_LANGID    (0xFFFF)

#define TEXTSERVICE_DESC    TEXT("TSF Property Monitor")
#define TEXTSERVICE_MODEL   TEXT("Apartment")

#define TEXTSERVICE_ICON_INDEX  0
#define LANGBAR_ITEM_DESC   TEXT("TSF Property Monitor")

extern HINSTANCE g_hInst;

extern LONG g_cRefDll;

extern CRITICAL_SECTION g_cs;

extern const CLSID c_clsidPropertyMonitorTextService;

extern const GUID c_guidProfile;

extern const GUID c_guidLangBarItemButton;


#endif // GLOBALS_H
