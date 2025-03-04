#include "Globals.h"
#include "TextService.h"
#include "PopupWindow.h"

TCHAR CPropertyPopupWindow::_szWndClass[] = TEXT("PropertyPopupWindow");

CPropertyPopupWindow::CPropertyPopupWindow()
{
    _hwnd = NULL;
    _psz = NULL;
}

CPropertyPopupWindow::~CPropertyPopupWindow()
{
    if (IsWindow(_hwnd))
        DestroyWindow(_hwnd);

    if (_psz)
        LocalFree(_psz);
}

BOOL CPropertyPopupWindow::StaticInit()
{

    WNDCLASSEX wcex;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize        = sizeof(wcex);
    wcex.style         = CS_HREDRAW | CS_VREDRAW ;
    wcex.hInstance     = g_hInst;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);

    wcex.lpfnWndProc   = _WndProc;
    wcex.lpszClassName = _szWndClass;
    RegisterClassEx(&wcex);

    return TRUE;
}


HWND CPropertyPopupWindow::CreateWnd()
{
    if (_hwnd)
        return _hwnd;
    // https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
    _hwnd = CreateWindowEx(WS_EX_TOPMOST, _szWndClass, TEXT("TSF Property Monitor"),
                           WS_POPUP | WS_THICKFRAME | WS_DISABLED,
                           0, 0, 0, 0,
                           NULL, 0, g_hInst, this);

    return _hwnd;
}

LRESULT CALLBACK CPropertyPopupWindow::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPropertyPopupWindow *_this;
    HDC hdc;
    PAINTSTRUCT ps;
  
    _this = _GetThis(hwnd);

    switch (uMsg)
    {
        case WM_CREATE:
            _SetThis(hwnd, lParam);
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            if (_this)
                _this->OnPaint(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


//+---------------------------------------------------------------------------
// Show the debugger window
//----------------------------------------------------------------------------
void CPropertyPopupWindow::Show()
{
    if (!IsWindow(_hwnd))
        return;

    if (!_psz|| !lstrlenW(_psz))
    {
        Hide();
        return;
    }

    RECT rcWork;
    SystemParametersInfo(SPI_GETWORKAREA,  0, &rcWork, FALSE);

    InvalidateRect(_hwnd, NULL, TRUE);
    SetWindowPos(_hwnd, 
                 HWND_TOPMOST, 
                 rcWork.right - POPUP_CX,
                 rcWork.bottom - POPUP_CY,
                 POPUP_CX,
                 POPUP_CY,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);

}

void CPropertyPopupWindow::Hide()
{
    if (!IsWindow(_hwnd))
        return;

    ShowWindow(_hwnd, SW_HIDE);
}

void CPropertyPopupWindow::SetString(IStream *pStream)
{
    if (_psz)
        LocalFree(_psz);

    LARGE_INTEGER ll;
    ll.QuadPart = 0;
    pStream->Seek(ll, STREAM_SEEK_SET, NULL);

    STATSTG stat;
    pStream->Stat(&stat, STATFLAG_NONAME);
    if (stat.cbSize.HighPart)
        return;

    _psz = (WCHAR *)LocalAlloc(LPTR, stat.cbSize.LowPart + sizeof(WCHAR));
    if (!_psz)
        return;

    pStream->Read(_psz, stat.cbSize.LowPart, NULL);

    ll.QuadPart = 0;
    pStream->Seek(ll, STREAM_SEEK_SET, NULL);

    InvalidateRect(_hwnd, NULL, TRUE);
}

void CPropertyPopupWindow::OnPaint(HWND hwnd, HDC hdc)
{
    HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);

    if (_psz)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        DrawTextW(hdc, _psz, lstrlenW(_psz), &rc, DT_EXPANDTABS);
    }

    SelectObject(hdc, hfontOld);
}

void CPropertyMonitorTextService::_ShowPopupWindow()
{
    if (!_pPopupWindow)
        _pPopupWindow = new CPropertyPopupWindow;

    if (_pPopupWindow)
    {
        _pPopupWindow->CreateWnd();
        _pPopupWindow->SetString(_pMemStream);
        _pPopupWindow->Show();
    }
}
