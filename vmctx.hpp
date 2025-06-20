#ifndef VMCTX_H
#define VMCTX_H
#include "vmmain.hpp"

namespace rvm64::context {
    _native void save_host_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->host_context))) {
        	CSR_SET_TRAP(nullptr, load_access_fault, status, 0, 1);
        }
    }

    _native void restore_host_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->host_context))) {
        	CSR_SET_TRAP(nullptr, load_access_fault, status, 0, 1);
        }
    }

    _native void save_vm_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
        	CSR_SET_TRAP(nullptr, load_access_fault, status, 0, 1);
        }
    }

    _native void restore_vm_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
        	CSR_SET_TRAP(nullptr, load_access_fault, status, 0, 1);
        }
    }
};
#endif //VMCTX_H
