#include "cbit.h"
#include <afxwin.h>

// Load and process the bitmap data.
void LoadAndProcessBitmap( TCHAR* pBitmapPath )
{
    // Load the image bitmapt.
    HBITMAP hBitmap = 0;
    hBitmap = (HBITMAP)LoadImage( NULL,
                                  pBitmapPath,
                                  IMAGE_BITMAP,
                                  0,
                                  0,
                                  LR_LOADFROMFILE | LR_DEFAULTSIZE);

    // Access bitmap data.
    CBitmap Bitmap;
    Bitmap.Attach( hBitmap );

    // Calculate buffer for bitmap bits.
    BITMAP BitmapInfo = { 0 };
    Bitmap.GetBitmap( &BitmapInfo );

    // Calculate the size of required buffer.
    DWORD BitmapImageSize = BitmapInfo.bmHeight *
        BitmapInfo.bmWidth *
        ( BitmapInfo.bmBitsPixel / 8 );

    // Allocate memory.
    BYTE* pBitmapData = new BYTE[ BitmapImageSize ];
    ZeroMemory( pBitmapData, BitmapImageSize );

    // Get bitmap data.
    Bitmap.GetBitmapBits( BitmapImageSize, pBitmapData );

    // Now access and process bitmap data
    // as you wish!

    // Now after processing, set the bitmap data back.
    Bitmap.SetBitmapBits( BitmapImageSize, pBitmapData );

    // Now you can use the processed bitmap for your purpose.
    // For instance, save to disk, display in your dialog etc.

    // Delete bitmap data after use.
    delete pBitmapData;
    pBitmapData = 0;
}
