// Copyright 2024 jeonghun

#include "nocd_patch.h"
#include "code_patcher.h"

namespace nocd {

uint32_t patch_target_addr_a = 0;
uint32_t patch_target_addr_b = 0;
uint32_t patch_target_addr_c = 0;

__declspec(naked) static void nocd_patch_hook_a() {
  __asm {
    pushad
    mov ecx, patch_target_addr_a
    mov eax, dword ptr [ecx]
    mov eax, dword ptr [eax+0Ch]
    mov ecx, 100h
    mov esi, patch_target_addr_b
    pushad
    mov edi, dword ptr [eax+24h]
    rep movs dword ptr es:[edi], dword ptr [esi]
    popad
    pushad
    mov edi, dword ptr [eax+34h]
    rep movs dword ptr es:[edi], dword ptr [esi]
    popad
    mov edi, dword ptr [eax+44h]
    rep movs dword ptr es:[edi], dword ptr [esi]
    popad
    ret
  }
}

__declspec(naked) static void nocd_patch_hook_b() {
  __asm {
    mov ecx, patch_target_addr_c
    mov eax, dword ptr [ecx]
    test eax, eax
    je RETURN
    lea ecx, [eax + 1504Ch]
    mov dword ptr [eax + 3894Ch], ecx
    lea eax, [eax + 34BAAh]
    mov patch_target_addr_b, eax
RETURN:
    push GetModuleFileNameA
    ret
  }
}

__declspec(naked) static void nocd_patch_install() {
  __asm {
    push eax
    push eax
    mov ebx, eax
    lea eax, [ebx + 8A064h]
    mov patch_target_addr_a, eax
    lea eax, [ebx + 7FC44h]
    mov patch_target_addr_c, eax
    mov dword ptr [ebx + 6C6B4h], 0
    mov eax, nocd_patch_hook_a
    mov dword ptr [ebx + 6C6BCh], eax
    lea ebx, [ebx + 571C8h]
    push esp
    push 4
    push 4
    push ebx
    call VirtualProtect
    mov eax, nocd_patch_hook_b
    mov dword ptr [ebx], eax
    pop eax
    pop eax
    ret
  }
}

bool patch() {
  constexpr uint64_t org_data = 0x00008095FF530775;
  uint8_t* target_ptr = reinterpret_cast<uint8_t*>(0x0157FC15);
  if (memcmp(target_ptr, &org_data, sizeof(org_data)) != 0)
      return false;

  uintptr_t fptr = reinterpret_cast<uintptr_t>(nocd_patch_install);
  uint8_t* addr = reinterpret_cast<uint8_t*>(&fptr);
  uint8_t patch_code[] = {
    0xB9, addr[0], addr[1], addr[2], addr[3],  // mov ecx, nocd_patch_install
    0xFF, 0xD1,                                // call ecx
    0x90,                                      // nop
    0x90                                       // nop
  };

  return code_patcher().apply(target_ptr, patch_code, sizeof(patch_code));
}

}  // namespace nocd
