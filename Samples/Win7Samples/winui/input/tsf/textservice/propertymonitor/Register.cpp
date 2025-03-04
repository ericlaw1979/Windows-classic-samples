// Register with TSF by updating the registry.
// https://learn.microsoft.com/en-us/windows/win32/tsf/text-service-registration
#include <windows.h>
#include <ole2.h>
#include "msctf.h"
#include "globals.h"

#define CLSID_STRLEN 38  // strlen("{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}")

static const TCHAR c_szInfoKeyPrefix[] = TEXT("CLSID\\");
static const TCHAR c_szInProcSvr32[] = TEXT("InProcServer32");
static const TCHAR c_szModelName[] = TEXT("ThreadingModel");

static const GUID kCategories[] = {
  GUID_TFCAT_TIP_KEYBOARD,              // It's a keyboard input method.
  GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,  // It supports inline input.
  GUID_TFCAT_TIPCAP_UIELEMENTENABLED,   // It supports UI less mode.
  // COM less is required for some applications, like WOW.
//  GUID_TFCAT_TIPCAP_COMLESS,
};

// https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfcategorymgr-registercategory
BOOL RegisterCategories()
{
    ITfCategoryMgr* pCategoryMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_ALL, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
    if (FAILED(hr)) return hr;
    for (int i = 0; i < ARRAYSIZE(kCategories); i++) {
        hr = pCategoryMgr->RegisterCategory(c_clsidPropertyMonitorTextService, kCategories[i], c_clsidPropertyMonitorTextService);
        if (FAILED(hr)) return hr;
    }
    return S_OK;
}

void UnregisterCategories() {
    ITfCategoryMgr* pCategoryMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_ALL, IID_ITfCategoryMgr, (void**)&pCategoryMgr);

    if (SUCCEEDED(hr)) {
        IEnumGUID *guids;
        hr = pCategoryMgr->EnumCategoriesInItem(c_clsidPropertyMonitorTextService, &guids);
        GUID guid = { 0 };
        ULONG fetched = 0;
        if (SUCCEEDED(hr)) {
            while (guids->Next(1, &guid, &fetched) == S_OK) {
                pCategoryMgr->UnregisterCategory(c_clsidPropertyMonitorTextService, guid, c_clsidPropertyMonitorTextService);
            }
        }
    }
}

BOOL RegisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    WCHAR achIconFile[MAX_PATH];
    int cchIconFile;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return E_FAIL;

    hr = pInputProcessProfiles->Register(c_clsidPropertyMonitorTextService);

    if (hr != S_OK)
        goto Exit;

    cchIconFile = GetModuleFileName(g_hInst, achIconFile, ARRAYSIZE(achIconFile));

    // https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-addlanguageprofile
    hr = pInputProcessProfiles->AddLanguageProfile(c_clsidPropertyMonitorTextService,
                                  TEXTSERVICE_LANGID,
                                  c_guidProfile,
                                  TEXTSERVICE_DESC,
                                  lstrlen(TEXTSERVICE_DESC),
                                  achIconFile,
                                  cchIconFile,
                                  TEXTSERVICE_ICON_INDEX);

Exit:
    pInputProcessProfiles->Release();
    return (hr == S_OK);
}

void UnregisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return;

    pInputProcessProfiles->Unregister(c_clsidPropertyMonitorTextService);
    pInputProcessProfiles->Release();
}

BOOL CLSIDToString(REFGUID refGUID, TCHAR *pchA)
{
    static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                   8, 9, '-', 10, 11, 12, 13, 14, 15};

    static const TCHAR szDigits[] = TEXT("0123456789ABCDEF");

    int i;
    TCHAR *p = pchA;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    *p++ = TEXT('{');
    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == TEXT('-'))
        {
            *p++ = TEXT('-');
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p++ = TEXT('}');
    *p   = TEXT('\0');

    return TRUE;
}

//+---------------------------------------------------------------------------
// RecurseDeleteKey is necessary because on NT RegDeleteKey doesn't work if the
// specified key has subkeys
//----------------------------------------------------------------------------
LONG RecurseDeleteKey(HKEY hParentKey, LPCTSTR lpszKey)
{
    HKEY hKey;
    LONG lRes;
    FILETIME time;
    TCHAR szBuffer[256];
    DWORD dwSize = ARRAYSIZE(szBuffer);

    if (RegOpenKey(hParentKey, lpszKey, &hKey) != ERROR_SUCCESS)
        return ERROR_SUCCESS; // let's assume we couldn't open it because it's not there

    lRes = ERROR_SUCCESS;
    while (RegEnumKeyEx(hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time)==ERROR_SUCCESS)
    {
        szBuffer[ARRAYSIZE(szBuffer)-1] = '\0';
        lRes = RecurseDeleteKey(hKey, szBuffer);
        if (lRes != ERROR_SUCCESS)
            break;
        dwSize = ARRAYSIZE(szBuffer);
    }
    RegCloseKey(hKey);

    return lRes == ERROR_SUCCESS ? RegDeleteKey(hParentKey, lpszKey) : lRes;
}

// Register with the system as an implementer of the contract.
BOOL RegisterServer()
{
    DWORD dw;
    HKEY hKey;
    HKEY hSubKey;
    BOOL fRet;
    TCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];
    TCHAR achFileName[MAX_PATH];

    if (!CLSIDToString(c_clsidPropertyMonitorTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return FALSE;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(TCHAR));

    if (fRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dw)
            == ERROR_SUCCESS)
    {
        fRet &= RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)TEXTSERVICE_DESC, (lstrlen(TEXTSERVICE_DESC)+1)*sizeof(TCHAR))
            == ERROR_SUCCESS;

        if (fRet &= RegCreateKeyEx(hKey, c_szInProcSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, &dw)
            == ERROR_SUCCESS)
        {
            dw = GetModuleFileName(g_hInst, achFileName, ARRAYSIZE(achFileName));

            fRet &= RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (BYTE *)achFileName, (lstrlen(achFileName)+1)*sizeof(TCHAR)) == ERROR_SUCCESS;
            fRet &= RegSetValueEx(hSubKey, c_szModelName, 0, REG_SZ, (BYTE *)TEXTSERVICE_MODEL, (lstrlen(TEXTSERVICE_MODEL)+1)*sizeof(TCHAR)) == ERROR_SUCCESS;
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    return fRet;
}


void UnregisterServer()
{
    TCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];

    if (!CLSIDToString(c_clsidPropertyMonitorTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(TCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}
