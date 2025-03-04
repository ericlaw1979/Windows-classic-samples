#include "Globals.h"
#include "TextService.h"
#include "Resource.h"
#include "PopupWindow.h"

// The cookie for the sink to CLangBarItemButton.
#define TEXTSERVICE_LANGBARITEMSINK_COOKIE 0x0fab0fab

// The id of the menu item of the language bar button.
#define MENUITEM_INDEX_SHOWPOPUPWINDOW 0

// The description of the menu item of the language bar button.
static WCHAR c_szMenuItemDescriptionShowPopupWindow[] = L"Show PopupWindow";

class CLangBarItemButton : public ITfLangBarItemButton,
                           public ITfSource
{
public:
    CLangBarItemButton(CPropertyMonitorTextService *pTextService);
    ~CLangBarItemButton();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfLangBarItem
    STDMETHODIMP GetInfo(TF_LANGBARITEMINFO *pInfo);
    STDMETHODIMP GetStatus(DWORD *pdwStatus);
    STDMETHODIMP Show(BOOL fShow);
    STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip);

    // ITfLangBarItemButton
    STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea);
    STDMETHODIMP InitMenu(ITfMenu *pMenu);
    STDMETHODIMP OnMenuSelect(UINT wID);
    STDMETHODIMP GetIcon(HICON *phIcon);
    STDMETHODIMP GetText(BSTR *pbstrText);

    // ITfSource
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

private:
    ITfLangBarItemSink *_pLangBarItemSink;
    TF_LANGBARITEMINFO _tfLangBarItemInfo;

    CPropertyMonitorTextService *_pTextService;
    LONG _cRef;
};

CLangBarItemButton::CLangBarItemButton(CPropertyMonitorTextService *pTextService)
{
    DllAddRef();

    // initialize TF_LANGBARITEMINFO structure.
    _tfLangBarItemInfo.clsidService = c_clsidPropertyMonitorTextService;    // This LangBarItem belongs to this TextService.
    _tfLangBarItemInfo.guidItem = c_guidLangBarItemButton;   // GUID of this LangBarItem.
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_BTN_MENU;      // This LangBar is a button type with a menu.
    _tfLangBarItemInfo.ulSort = 0;                           // The position of this LangBar Item is not specified.
    StringCchCopy(_tfLangBarItemInfo.szDescription, ARRAYSIZE(_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);                        // Set the description of this LangBar Item.

    // Initialize the sink pointer.
    _pLangBarItemSink = NULL;

    _pTextService = pTextService;
    _pTextService->AddRef();

    _cRef = 1;
}

CLangBarItemButton::~CLangBarItemButton()
{
    DllRelease();
    _pTextService->Release();
}

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem) ||
        IsEqualIID(riid, IID_ITfLangBarItemButton))
    {
        *ppvObj = (ITfLangBarItemButton *)this;
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = (ITfSource *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CLangBarItemButton::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

STDAPI CLangBarItemButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
    *pInfo = _tfLangBarItemInfo;
    return S_OK;
}

STDAPI CLangBarItemButton::GetStatus(DWORD *pdwStatus)
{
    *pdwStatus = 0;
    return S_OK;
}

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
    return E_NOTIMPL;
}

STDAPI CLangBarItemButton::GetTooltipString(BSTR *pbstrToolTip)
{
    *pbstrToolTip = SysAllocString(LANGBAR_ITEM_DESC);

    return (*pbstrToolTip == NULL) ? E_OUTOFMEMORY : S_OK;
}

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
    return S_OK;
}

// Add our menu item to the language bar, showing as checked if the
// popup window is showing.
STDAPI CLangBarItemButton::InitMenu(ITfMenu *pMenu)
{
    DWORD dwFlags = 0;
    if (_pTextService->_GetPopupWindow() &&
        _pTextService->_GetPopupWindow()->IsShown())
    {
        dwFlags |= TF_LBMENUF_CHECKED;
    }

    pMenu->AddMenuItem(MENUITEM_INDEX_SHOWPOPUPWINDOW,
                       dwFlags,
                       NULL,
                       NULL,
                       c_szMenuItemDescriptionShowPopupWindow,
                       (ULONG)lstrlen(c_szMenuItemDescriptionShowPopupWindow), 
                       NULL);

    return S_OK;
}

// This callback runs when the menu item is invoked.
STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
    switch (wID)
    {
        case MENUITEM_INDEX_SHOWPOPUPWINDOW:
            OutputDebugString(L"!!!TSF Debugger: Popup window requested");
            if (_pTextService->_GetPopupWindow())
            {
                if (_pTextService->_GetPopupWindow()->IsShown())
                    _pTextService->_GetPopupWindow()->Hide();
                else
                    _pTextService->_GetPopupWindow()->Show();
            }
            else {
                OutputDebugString(L"TSF Debugger: failed to get popup window.");
            }
            break;
    }

    return S_OK;
}

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
    *phIcon = (HICON)LoadImage(g_hInst, TEXT("IDI_TEXTSERVICE"), IMAGE_ICON, 16, 16, 0);
 
    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

STDAPI CLangBarItemButton::GetText(BSTR *pbstrText)
{
    *pbstrText = SysAllocString(LANGBAR_ITEM_DESC);

    return (*pbstrText == NULL) ? E_OUTOFMEMORY : S_OK;
}

STDAPI CLangBarItemButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    // We allow only ITfLangBarItemSink interface.
    if (!IsEqualIID(IID_ITfLangBarItemSink, riid))
        return CONNECT_E_CANNOTCONNECT;

    // We support only one sink once.
    if (_pLangBarItemSink != NULL)
        return CONNECT_E_ADVISELIMIT;

    // Query the ITfLangBarItemSink interface and store it into _pLangBarItemSink.
    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = NULL;
        return E_NOINTERFACE;
    }

    // return our cookie.
    *pdwCookie = TEXTSERVICE_LANGBARITEMSINK_COOKIE;
    return S_OK;
}

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
    // Check the given cookie.
    if (dwCookie != TEXTSERVICE_LANGBARITEMSINK_COOKIE)
        return CONNECT_E_NOCONNECTION;

    // If there is no connected sink, fail.
    if (_pLangBarItemSink == NULL)
        return CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release();
    _pLangBarItemSink = NULL;

    return S_OK;
}

BOOL CPropertyMonitorTextService::_InitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;
    BOOL fRet;

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) != S_OK)
        return FALSE;

    fRet = FALSE;

    if ((_pLangBarItem = new CLangBarItemButton(this)) == NULL)
        goto Exit;

    if (pLangBarItemMgr->AddItem(_pLangBarItem) != S_OK)
    {
        _pLangBarItem->Release();
        _pLangBarItem = NULL;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    pLangBarItemMgr->Release();
    return fRet;
}

void CPropertyMonitorTextService::_UninitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;

    if (_pLangBarItem == NULL)
        return;

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) == S_OK)
    {
        pLangBarItemMgr->RemoveItem(_pLangBarItem);
        pLangBarItemMgr->Release();
    }

    _pLangBarItem->Release();
    _pLangBarItem = NULL;
}
