#include "vmmain.hpp"
#include "vmctx.h"
#include "vmcs.h"

namespace rvm64::memory {
    __function void vm_init(void) {
        rvm64::context::vm_context_start();

        vmcs.program_size = PROCESS_MAX_CAPACITY;
        vmcs.load_rsv_valid = false;
        vmcs.load_rsv_addr = 0LL;

        if (!NT_SUCCESS(vmcs.reason = ctx->win32.NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&vmcs.program, 0, &vmcs.program_size,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
            vmcs.halt = 1;
        }
    }

    __function void vm_end(void) {
        if (!NT_SUCCESS(vmcs.reason = ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (void**)&vmcs.program, &vmcs.program_size, MEM_RELEASE))) {
            vmcs.halt = 1;
        }
    }

    __function void vm_set_load_rsv(uintptr_t address) {
        vmcs.load_rsv_addr = address;
        vmcs.load_rsv_valid = true;
    }

    __function void vm_clear_load_rsv(void) {
        vmcs.load_rsv_addr = 0LL;
        vmcs.load_rsv_valid = false;
    }

    __function bool vm_check_load_rsv(uintptr_t address) {
        return vmcs.load_rsv_valid && vmcs.load_rsv_addr == address;
    }

    __function void vm_stkchk(uintptr_t sp) {
        uintptr_t stack_start = *(uintptr_t*)vmcs.vregs[regenum::sp];
        uintptr_t stack_end = *(uintptr_t*)vmcs.vregs[regenum::sp] + VSTACK_MAX_CAPACITY;

        if (sp < stack_start || sp >= stack_end) {
            vmcs.halt = 1;
            vmcs.reason = access_violation;
            return;
        }

        uintptr_t _ra = 0; // TODO: shadow_call_stack_pop();
        if (vmcs.vregs[regenum::ra] != _ra) {
            vmcs.halt = 1;
            vmcs.reason = return_address_corruption;
        }

        uintptr_t canary1 = *(uintptr_t*)sp ;
        uintptr_t canary2 = *(uintptr_t*)(sp + sizeof(uintptr_t));

        if (canary1 != __stack_cookie || canary2 != __stack_cookie) {
            vmcs.halt = 1;
            vmcs.reason = stack_overflow;
        }
    }
};