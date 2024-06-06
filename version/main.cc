// Copyright 2024 jeonghun

#include <windows.h>
#include "dll_loader.h"
#include "nocd_patch.h"
#include "secdrv_emulation.h"

static dll_loader version(L"version", true);

DWORD WINAPI GetFileVersionInfoSizeA(
    LPCSTR lptstrFilename, LPDWORD lpdwHandle) {
  return version.call<decltype(&GetFileVersionInfoSizeA)>(
      "GetFileVersionInfoSizeA", lptstrFilename, lpdwHandle);
}

BOOL APIENTRY GetFileVersionInfoA(
    LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
  return version.call<decltype(&GetFileVersionInfoA)>("GetFileVersionInfoA",
      lptstrFilename, dwHandle, dwLen, lpData);
}

BOOL APIENTRY VerQueryValueA(
    LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen) {
  return version.call<decltype(&VerQueryValueA)>(
      "VerQueryValueA", pBlock, lpSubBlock, lplpBuffer, puLen);
}

int APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
  switch (Reason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hDLL);
      nocd::patch();
      secdrv::emulate();
      break;

    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}
