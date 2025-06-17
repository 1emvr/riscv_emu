#ifndef VMMEM_H
#define VMMEM_H
#include "vmmain.hpp"
#include "vmelf.hpp"

namespace rvm64::memory {
    _native void context_init() {
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

    _vmcall void vm_set_load_rsv(int hart_id, uintptr_t address) {
        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = address; // vmcs_array[hart_id]->load_rsv_addr = address;
        vmcs->load_rsv_valid = true; // vmcs_array[hart_id]->load_rsv_valid = true;

        ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    _vmcall void vm_clear_load_rsv(int hart_id) {
        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = 0LL; // vmcs_array[hart_id]->load_rsv_addr = 0LL;
        vmcs->load_rsv_valid = false; // vmcs_array[hart_id]->load_rsv_valid = false;

        ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    _vmcall bool vm_check_load_rsv(int hart_id, uintptr_t address) {
        int valid = 0;

        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);
        valid = (vmcs->load_rsv_valid && vmcs->load_rsv_addr == address); // (vmcs_array[hart_id]->load_rsv_valid && vmcs_array[hart_id]->load_rsv_addr == address)

        ctx->win32.NtReleaseMutex(vmcs_mutex);
        return valid;
    }

	_native bool memory_init(size_t process_size) {
    	NTSTATUS status = 0;
		vmcs->process.size = process_size;

	    if (!NT_SUCCESS(status = ctx->win32.NtAllocateVirtualMemory(NtCurrentProcess(), (LPVOID*) &vmcs->process.address, 0,
		    &vmcs->process.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {

		    csr_set(nullptr, load_access_fault, status, 0, 1);
		    return false;
	    }
		return true;
	}

	_native void memory_end() {
		vmcs->reason = ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID*)&vmcs->process.address, &vmcs->process.size, MEM_RELEASE);
	}
};

namespace rvm64::context {
    _native void save_host_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->host_context))) {
        	csr_set(nullptr, load_access_fault, status, 0, 1);
        }
    }

    _native void restore_host_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->host_context))) {
        	csr_set(nullptr, load_access_fault, status, 0, 1);
        }
    }

    _native void save_vm_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
        	csr_set(nullptr, load_access_fault, status, 0, 1);
        }
    }

    _native void restore_vm_context() {
    	NTSTATUS status = 0;
        if (!NT_SUCCESS(status = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
        	csr_set(nullptr, load_access_fault, status, 0, 1);
        }
    }
};
#endif // VMMEM_H
