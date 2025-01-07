#ifndef WIN_MAIN_H
#define WIN_MAIN_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

INT WINAPI wWinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE prev_instance,
	_In_ PWSTR cmd_line,
	_In_ INT cmd_show
);

#endif // WIN_MAIN_H
