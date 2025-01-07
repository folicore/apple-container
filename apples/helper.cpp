#include "helper.h"

HRESULT help::hCheck(HRESULT hresultVal) {
	if (hresultVal >= 0) {
		return hresultVal;
	} else {
		throw hresultNotOk(hresultVal);
	}
}


UINT64 help::myTimer64ms() {
	static LARGE_INTEGER freq = { .QuadPart = 0 };
	if (freq.QuadPart == 0) {
		if (!QueryPerformanceFrequency(&freq)) { throw std::exception(); }
	}

	LARGE_INTEGER time;
	if (!QueryPerformanceCounter(&time)) { throw std::exception(); }

	return 1000 * time.QuadPart / freq.QuadPart;
}

UINT64 help::myTimer64us() {
	static LARGE_INTEGER freq = { .QuadPart = 0 };
	if (freq.QuadPart == 0) {
		if (!QueryPerformanceFrequency(&freq)) { throw std::exception(); }
	}

	LARGE_INTEGER time;
	if (!QueryPerformanceCounter(&time)) { throw std::exception(); }

	// Mutliplying it by 10^6 will porbably cause overflow after aroud 1.5 motnts of running a PC
	// for frequency of 10^7
	return 1'000'000 * time.QuadPart / freq.QuadPart;
}