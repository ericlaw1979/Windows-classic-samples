
#include "Globals.h"

IStream* CreateMemoryStream()
{
    LPSTREAM lpStream = NULL;

    // Create a stream object on a memory block.
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, 0);
    if (hGlobal != NULL)
    {
        HRESULT hr ;
        if (FAILED(hr = CreateStreamOnHGlobal(hGlobal, TRUE, &lpStream)))
        {
             GlobalFree(hGlobal);
        }
    }

    return lpStream;
}

void ClearStream(IStream *pStream)
{
    ULARGE_INTEGER ull;
    ull.QuadPart = 0;
    pStream->SetSize(ull);
}

void AddStringToStream(IStream *pStream, WCHAR *psz)
{
    pStream->Write(psz, lstrlenW(psz) * sizeof(WCHAR), NULL);
}

