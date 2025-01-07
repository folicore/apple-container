// copied from https://learn.microsoft.com/en-us/windows/win32/Direct2D/how-to-load-a-direct2d-bitmap-from-a-file
#pragma once

#include "bitmapFileLoader.h"

using help::hCheck;

ID2D1Bitmap* LoadBitmapFromFile(
    ID2D1RenderTarget* pRenderTarget,
    IWICImagingFactory* pIWICFactory,
    PCWSTR uri
) {
    return LoadBitmapFromFile(pRenderTarget, pIWICFactory, uri, 0, 0);
}

ID2D1Bitmap* LoadBitmapFromFile(
    ID2D1RenderTarget* pRenderTarget,
    IWICImagingFactory* pIWICFactory,
    PCWSTR uri,
    UINT destinationWidth, // unused, but was in original code
    UINT destinationHeight // unused, but was in original code
) {
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pSource = nullptr;
    //IWICStream* pStream = nullptr; // unused, but was in original code
    IWICFormatConverter* pConverter = nullptr;
    IWICBitmapScaler* pScaler = nullptr;
    ID2D1Bitmap* ppBitmap = nullptr;


    hCheck(pIWICFactory->CreateDecoderFromFilename(
        uri, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &pDecoder));

    // Create the initial frame.
    hCheck(pDecoder->GetFrame(0, &pSource));

    // Convert the image format to 32bppPBGRA
    // (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
    hCheck(pIWICFactory->CreateFormatConverter(&pConverter));

    hCheck(pConverter->Initialize(
        pSource, GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr, 0.f, WICBitmapPaletteTypeMedianCut));

    // Create a Direct2D bitmap from the WIC bitmap.
    hCheck(pRenderTarget->CreateBitmapFromWicBitmap(
        pConverter, NULL, &ppBitmap));


    help::SafeRelease(pDecoder);
    help::SafeRelease(pSource);
    //help::SafeRelease(pStream); // unused, but was in original code
    help::SafeRelease(pConverter);
    help::SafeRelease(pScaler);

    return ppBitmap;
}
