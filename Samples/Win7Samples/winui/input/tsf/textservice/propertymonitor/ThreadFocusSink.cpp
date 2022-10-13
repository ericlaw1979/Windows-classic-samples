// ITfThreadFocusSink interface
#include "Globals.h"
#include "TextService.h"
#include "PopupWindow.h"

STDAPI CPropertyMonitorTextService::OnSetThreadFocus()
{
    OutputDebugString(L"TSF: CPropertyMonitorTextService::OnSetThreadFocus()");
    if (_pPopupWindow)
    {
       _pPopupWindow->Show();
    }
    return S_OK;
}

STDAPI CPropertyMonitorTextService::OnKillThreadFocus()
{
    OutputDebugString(L"TSF: CPropertyMonitorTextService::OnKillThreadFocus()");
    if (_pPopupWindow)
    {
       _pPopupWindow->Hide();
    }

    return S_OK;
}

BOOL CPropertyMonitorTextService::_InitThreadFocusSink()
{
    ITfSource *pSource;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &_dwThreadFocusCookie);
        pSource->Release();
    }

    return TRUE;
}

void CPropertyMonitorTextService::_UninitThreadFocusSink()
{
    ITfSource *pSource;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(_dwThreadFocusCookie);
        pSource->Release();
    }
}
