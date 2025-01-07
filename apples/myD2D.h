#pragma once

#include <d2d1_3.h>
#include <dwrite_3.h>

enum class rtd { // render target dependency
	NO_RENDER_TARGET_DEPENDENT,
	ONLY_RENDER_TARGET_DEPENDENT,
	ALL,
};

struct MyD2DObjectCollection {
	ID2D1Factory7* d2d_factory = nullptr;
	IDWriteFactory5* write_factory = nullptr;
	IWICImagingFactory* imaging_factory = nullptr;

	ID2D1HwndRenderTarget* d2d_render_target = nullptr;

	void init(HWND hwnd, rtd rtdv);
	void free(rtd rtdv);
};
