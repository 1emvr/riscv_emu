#ifndef VMCTX_H
#define VMCTX_H
#include "vmmain.hpp"

namespace rvm64::context {
    _function void vm_context_init() {
        // Fake implant context
        ctx = (hexane*) VirtualAlloc(nullptr, sizeof(ctx), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        ctx->win32.RtlAllocateHeap = (RtlAllocateHeap_t) GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlAllocateHeap");
        ctx->win32.NtGetContextThread = (NtGetContextThread_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtGetContextThread");
        ctx->win32.NtSetContextThread = (NtSetContextThread_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtSetContextThread");
        ctx->win32.NtAllocateVirtualMemory = (NtAllocateVirtualMemory_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtAllocateVirtualMemory");
        ctx->win32.NtFreeVirtualMemory = (NtFreeVirtualMemory_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtFreeVirtualMemory");
        ctx->win32.NtGetFileSize = (decltype(GetFileSize) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFileSize");
        ctx->win32.NtReadFile = (decltype(ReadFile) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "ReadFile");
        ctx->win32.NtCreateFile = (decltype(CreateFileA) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileA");
        ctx->win32.NtCreateMutex = (decltype(CreateMutexA) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateMutexA");
        ctx->win32.NtReleaseMutex = (decltype(ReleaseMutex) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "ReleaseMutex");
        ctx->win32.NtWaitForSingleObject = (decltype(WaitForSingleObject) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "WaitForSingleObject");
    }

    _function void save_host_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->host_context))) {
            vmcs->halt = 1;
        }
    }

    _function void restore_host_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->host_context))) {
            vmcs->halt = 1;
        }
    }

    _function void save_vm_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
            vmcs->halt = 1;
        }
    }

    _function void restore_vm_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
            vmcs->halt = 1;
        }
    }
};
#endif // VMCTX_H
