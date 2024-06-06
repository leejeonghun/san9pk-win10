// Copyright 2024 jeonghun

#include "secdrv_emulation.h"
#include <ntstatus.h>
#include <winternl.h>
#include <cassert>
#include "func_hooker.h"
#include "../third_party/SafeDiscShim/src/secdrv_ioctl.h"

using namespace secdrvIoctl;

namespace secdrv {

func_hooker createfilea;
HANDLE WINAPI hook_createfilea(LPCSTR lpFileName, DWORD dwDesiredAccess,
    DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile) {
  func_hooker::rehook_on_exit rehook(createfilea);
  if (lstrcmpiA(lpFileName, "\\\\.\\Global\\SecDrv") == 0) {
    return CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  }
  return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
      lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
      hTemplateFile);
}

decltype(&NtDeviceIoControlFile) ntdeviceiocontrolfile_ptr = nullptr;
func_hooker ntdeviceiocontrolfile;
NTSTATUS NTAPI hook_ntdeviceiocontrolfile(HANDLE FileHandle, HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer,
    ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength) {
  func_hooker::rehook_on_exit rehook(ntdeviceiocontrolfile);
  if (IoControlCode == ioctlCodeMain) {
    if (ProcessMainIoctl(
            InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength)) {
      IoStatusBlock->Information = OutputBufferLength;
      IoStatusBlock->Status = STATUS_SUCCESS;
    } else
      IoStatusBlock->Status = STATUS_UNSUCCESSFUL;
    return IoStatusBlock->Status;
  } else {
    return ntdeviceiocontrolfile_ptr(FileHandle, Event, ApcRoutine, ApcContext,
        IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength,
        OutputBuffer, OutputBufferLength);
  }
}

func_hooker createwindowexa;
HWND WINAPI hook_createwindowexa(DWORD dwExStyle, LPCSTR lpClassName,
    LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
  func_hooker::rehook_on_exit rehook(createwindowexa);
  if (lstrcmpiA(lpClassName, "dummy") != 0) {
    rehook.cancel();
    ntdeviceiocontrolfile.unhook();
    createfilea.unhook();
  }
  return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y,
      nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

bool emulate() {
  ntdeviceiocontrolfile_ptr =
      reinterpret_cast<decltype(ntdeviceiocontrolfile_ptr)>(
          GetProcAddress(GetModuleHandle(L"ntdll"), "NtDeviceIoControlFile"));
  assert(ntdeviceiocontrolfile_ptr != nullptr);

  return createfilea.install(CreateFileA, hook_createfilea) &&
         createwindowexa.install(CreateWindowExA, hook_createwindowexa) &&
         ntdeviceiocontrolfile.install(
             ntdeviceiocontrolfile_ptr, hook_ntdeviceiocontrolfile);
}

}  // namespace secdrv
