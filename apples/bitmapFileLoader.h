#pragma once

#include <Windows.h>
#include "myD2D.h"
#include "wincodec.h"
#include "helper.h"

ID2D1Bitmap* LoadBitmapFromFile(
    ID2D1RenderTarget* pRenderTarget,
    IWICImagingFactory* pIWICFactory,
    PCWSTR uri
);

ID2D1Bitmap* LoadBitmapFromFile(
    ID2D1RenderTarget* pRenderTarget,
    IWICImagingFactory* pIWICFactory,
    PCWSTR uri,
    UINT destinationWidth,
    UINT destinationHeight
);
