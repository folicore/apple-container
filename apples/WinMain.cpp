#include "WinMain.h"

#include <Windows.h>
#include <exception>
#include <optional>
#include <windowsx.h>
#include "myD2D.h"
#include "helper.h"
#include "controller.h"

using help::hCheck;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void inbetweenFrames(HWND hwnd);

INT WINAPI wWinMain(
	_In_ [[maybe_unused]] HINSTANCE instance,
	_In_opt_ [[maybe_unused]] HINSTANCE prev_instance,
	_In_ [[maybe_unused]] PWSTR cmd_line,
	_In_ [[maybe_unused]] INT cmd_show
) {
	const wchar_t CLASS_NAME[] = L"Sample Window Class";
	WNDCLASSEX window = {
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = (WNDPROC)WindowProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = instance,
		.hIcon = nullptr,
		.hCursor = nullptr,
		.hbrBackground = nullptr,
		.lpszMenuName = nullptr,
		.lpszClassName = CLASS_NAME,
		.hIconSm = nullptr,
	};
	RegisterClassEx(&window);

	HWND hwnd = CreateWindowEx(
		0,									// Optional window styles.
		CLASS_NAME,							// Window class
		TEXT("Pou"),						// Window text
		WS_OVERLAPPEDWINDOW,				// Window style

		// Size and position:
		CW_USEDEFAULT,	// X
		CW_USEDEFAULT,	// Y
		CW_USEDEFAULT,	// nWidth
		CW_USEDEFAULT,	// nHeight

		nullptr,		// Parent window    
		nullptr,		// Menu
		instance,		// Instance handle
		nullptr			// Additional application data
	);

	if (hwnd == nullptr) {
		exit(1);
	}

	ShowWindow(hwnd, cmd_show);

	/*MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/

	MSG msg = { };
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message != WM_QUIT) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				//WindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
			}
		} else {
			inbetweenFrames(hwnd);
		}
	} while (msg.message != WM_QUIT);

	return 0;
}


namespace {
	bool initDone = false;
} // namepsace

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static std::optional<UINT64> last_time;
	static MyD2DObjectCollection myd2d;
	static Controller controller;

	controller.processWindowMsg(hwnd, uMsg, wParam, lParam);

	switch (uMsg) {
	case WM_CREATE:
		hCheck(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
		myd2d.init(hwnd, rtd::ALL);
		//pou::init(myd2d, rtd::ALL);
		initDone = true;
	return 0;

	case WM_SIZE: {
		UINT new_x = LOWORD(lParam);
		UINT new_y = HIWORD(lParam);
		myd2d.d2d_render_target->Resize(D2D1::SizeU(new_x, new_y));
	} return 0;

	case WM_PAINT: {
		UINT64 time = help::myTimer64ms();

		controller.pollAllKeys();

		myd2d.d2d_render_target->BeginDraw();
		//pou::draw(myd2d, time, last_time.value_or(0), controller);
		try {
			hCheck(myd2d.d2d_render_target->EndDraw());
			//if (time % 250 == 0) { throw hresultNotOk(D2DERR_RECREATE_TARGET); }
			// if render target is recrated, background W should reset
		} catch (help::hresultNotOk& e) {
			if (e.hresult == D2DERR_RECREATE_TARGET) {
				myd2d.free(rtd::ONLY_RENDER_TARGET_DEPENDENT);
				//pou::free(rtd::ONLY_RENDER_TARGET_DEPENDENT);

				myd2d.init(hwnd, rtd::ONLY_RENDER_TARGET_DEPENDENT);
				//pou::init(myd2d, rtd::ONLY_RENDER_TARGET_DEPENDENT);
			} else {
				throw;
			}
		}

		ValidateRect(hwnd, nullptr);
		last_time = time;
	} return 0;

	case WM_CLOSE:
		switch (MessageBox(nullptr, TEXT("Do you want to close this window?"), TEXT("Confirm closing window"), MB_YESNO | MB_ICONSTOP)) {
		case IDYES:
			return WindowProc(hwnd, WM_DESTROY, wParam, lParam);
		}
	return 0;

	case WM_DESTROY:
		//pou::free(rtd::ALL);
		myd2d.free(rtd::ALL);
		PostQuitMessage(0);
	return 0;

	// default:
		// keep DefWindowProc outside of switch, so if any of the cases forgets to return from function KWP is still called
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void inbetweenFrames(HWND hwnd) {
	const UINT64 MAX_FPS = 144;
	static UINT64 last_frame = 0;

	UINT64 time_ms = help::myTimer64ms();
	UINT64 current_frame = (time_ms * MAX_FPS) / 1000;

	if (current_frame != last_frame && initDone) {
		InvalidateRect(hwnd, nullptr, true);
	}

	last_frame = current_frame;
}