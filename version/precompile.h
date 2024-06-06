// Copyright 2016 jeonghun

#ifndef SAN9PK_WIN10_VERSION_PRECOMPILE_H_
#define SAN9PK_WIN10_VERSION_PRECOMPILE_H_

#define _WIN32_WINNT 0x0602
#define NTDDI_VERSION 0x06020000
#define UMDF_USING_NTSTATUS

#include <sdkddkver.h>
#include <windows.h>
#include <winsdkver.h>
#include <ntstatus.h>
#include <winternl.h>
#include <cstdint>

#endif  // SAN9PK_WIN10_VERSION_PRECOMPILE_H_
