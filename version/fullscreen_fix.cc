// Copyright 2024 jeonghun

#include "fullscreen_fix.h"
#include <ddraw.h>
#include "dll_loader.h"

namespace fullscreen {

bool fix(HWND hwnd) {
  static dll_loader ddraw(L"ddraw.dll", true);

  IDirectDraw* ddraw_ptr = nullptr;
  if (SUCCEEDED(ddraw.call<decltype(&DirectDrawCreate)>(
      "DirectDrawCreate", nullptr, &ddraw_ptr, nullptr))) {
    HRESULT hr = ddraw_ptr->SetCooperativeLevel(
        hwnd, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_NOWINDOWCHANGES);
    if (FAILED(hr))
        return false;

    hr = ddraw_ptr->SetDisplayMode(1024, 768, 32);
    return SUCCEEDED(hr);
  }

  return true;
}

}  // namespace fullscreen
