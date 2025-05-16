#include "vmcs.h"
namespace rvm64::context {
    __function void vm_entry(void) {
        // TODO
    }

    __function void vm_context_init() {
        ctx = (hexane*) VirtualAlloc(nullptr, sizeof(ctx), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        ctx->win32.NtGetContextThread = (NtGetContextThread_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtGetContextThread");
        ctx->win32.NtSetContextThread = (NtSetContextThread_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtSetContextThread");
        ctx->win32.NtAllocateVirtualMemory = (NtAllocateVirtualMemory_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtAllocateVirtualMemory");
        ctx->win32.NtFreeVirtualMemory = (NtFreeVirtualMemory_t) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtFreeVirtualMemory");
        ctx->win32.NtGetFileSize = (decltype(GetFileSize) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFileSize");
        ctx->win32.NtReadFile = (decltype(ReadFile) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "ReadFile");
        ctx->win32.NtCreateFile = (decltype(CreateFileA) *) GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileA");
    }

    __function void save_host_context(void) {
        if (!NT_SUCCESS(vmcs.reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs.host_context))) {
            vmcs.halt = 1;
        }
    }

    __function void restore_host_context(void) {
        if (!NT_SUCCESS(vmcs.reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs.host_context))) {
            vmcs.halt = 1;
        }
    }

    __function void save_vm_context(void) {
        if (!NT_SUCCESS(vmcs.reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs.vm_context))) {
            vmcs.halt = 1;
        }
    }

    __function void restore_vm_context(void) {
        if (!NT_SUCCESS(vmcs.reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs.vm_context))) {
            vmcs.halt = 1;
        }
    }
};
