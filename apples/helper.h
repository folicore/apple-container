// general purpose helper library
#pragma once

#include <Windows.h>
#include <d2d1_3.h>
#include <utility>
#include <vector>
#include <exception>


#define PI 3.141592657

namespace help {
	template <typename T>
	bool SafeRelease(T*& ptr) {
		if (ptr != nullptr) {
			ptr->Release();
			ptr = nullptr;
			return true;
		} else {
			return false;
		}
	}

	HRESULT hCheck(HRESULT hresultVal);
	class hresultNotOk : std::exception {
	public:
		HRESULT hresult;

		hresultNotOk(HRESULT hresultVal) {
			this->hresult = hresultVal;
		}
	};

	UINT64 myTimer64ms();
	UINT64 myTimer64us();
} // namespace help
