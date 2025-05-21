#ifndef VMMEM_H
#define VMMEM_H

#include "monodef.hpp"
#include "vmctx.hpp"
#include "vmmain.hpp"
#include "vmops.h"

namespace rvm64::memory {
    __function void vm_init() {
        rvm64::context::vm_context_init();

        vmcs->handler = (uintptr_t)operation::__handler;
        vmcs->dkey = __key; // lol idk...

        vmcs->process.size = PROCESS_MAX_CAPACITY;
        vmcs->load_rsv_valid = false;
        vmcs->load_rsv_addr = 0LL;

        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&vmcs->process.address, 0, &vmcs->process.size,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
            vmcs->halt = 1;
        }
    }

    __function void vm_end() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (void**)&vmcs->process.address, &vmcs->process.size, MEM_RELEASE))) {
            vmcs->halt = 1;
        }
    }

    __function void vm_set_load_rsv(/*int hart_id, */uintptr_t address) {
    	ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = address; // vmcs_array[hart_id]->load_rsv_addr = address;
        vmcs->load_rsv_valid = true; // vmcs_array[hart_id]->load_rsv_valid = true;

    	ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    __function void vm_clear_load_rsv(/*int hart_id, */) {
    	ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = 0LL; // vmcs_array[hart_id]->load_rsv_addr = 0LL;
        vmcs->load_rsv_valid = false; // vmcs_array[hart_id]->load_rsv_valid = false;

    	ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    __function bool vm_check_load_rsv(/*int hart_id, */uintptr_t address) {
    	int valid = 0;

    	ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);
        valid = (vmcs->load_rsv_valid && vmcs->load_rsv_addr == address); // (vmcs_array[hart_id]->load_rsv_valid && vmcs_array[hart_id]->load_rsv_addr == address)

    	ctx->win32.NtReleaseMutex(vmcs_mutex);
    	return valid;
    }
};
#endif // VMMEM_H
