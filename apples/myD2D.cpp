#include "myD2D.h"

#include "helper.h"
#include "wincodec.h"

using help::hCheck;

void MyD2DObjectCollection::init(HWND hwnd, rtd rtdv) {
	if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
		if (write_factory == nullptr) {
			hCheck(DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&write_factory)));
		}

		if (d2d_factory == nullptr) {
			hCheck(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory));
		}

		if (imaging_factory == nullptr) {
			hCheck(CoCreateInstance(
				CLSID_WICImagingFactory,
				nullptr,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&imaging_factory)));
		}
	}

	if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
		if (d2d_render_target == nullptr) {
			RECT rc;
			if (!GetClientRect(hwnd, &rc)) { throw std::exception(); }
			//rc = { .left = 0, .top = 0, .right = 1920, .bottom = 1080 };

			UINT32 size_x = rc.right - rc.left;
			UINT32 size_y = rc.bottom - rc.top;

			hCheck(d2d_factory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(size_x, size_y)),
				&d2d_render_target));
		}
	}
}

void MyD2DObjectCollection::free(rtd rtdv) {
	if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
		help::SafeRelease(d2d_factory);
		help::SafeRelease(write_factory);
		help::SafeRelease(imaging_factory);
	}

	if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
		help::SafeRelease(d2d_render_target);
	}
}
