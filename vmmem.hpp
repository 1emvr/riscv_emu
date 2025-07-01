#ifndef VMMEM_H
#define VMMEM_H
#include "vmmain.hpp"
#include "vmelf.hpp"

namespace rvm64::memory {
    _vmcall void vm_set_load_rsv(int hart_id, uintptr_t address) {
        WaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = address; // vmcs_array[hart_id]->load_rsv_addr = address;
        vmcs->load_rsv_valid = true; // vmcs_array[hart_id]->load_rsv_valid = true;

        ReleaseMutex(vmcs_mutex);
    }

    _vmcall void vm_clear_load_rsv(int hart_id) {
        WaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = 0LL; // vmcs_array[hart_id]->load_rsv_addr = 0LL;
        vmcs->load_rsv_valid = false; // vmcs_array[hart_id]->load_rsv_valid = false;

        ReleaseMutex(vmcs_mutex);
    }

    _vmcall bool vm_check_load_rsv(int hart_id, uintptr_t address) {
        int valid = 0;

        WaitForSingleObject(vmcs_mutex, INFINITE);
        valid = (vmcs->load_rsv_valid && vmcs->load_rsv_addr == address); // (vmcs_array[hart_id]->load_rsv_valid && vmcs_array[hart_id]->load_rsv_addr == address)

        ReleaseMutex(vmcs_mutex);
        return valid;
    }

	_native bool memory_init(size_t process_size) {
    	NTSTATUS status = 0;
		vmcs->process.size = process_size;

	    vmcs->process.address = (uintptr_t)VirtualAlloc(nullptr, vmcs->process.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if ((void*)vmcs->process.address == nullptr) {

		    CSR_SET_TRAP(nullptr, load_access_fault, status, 0, 1);
		    return false;
	    }
		return true;
	}

	_native void memory_end() {
    	NTSTATUS status = 0;
		if (!NT_SUCCESS(VirtualFree((LPVOID)vmcs->process.address, vmcs->process.size, MEM_RELEASE))) {
			CSR_SET_TRAP(nullptr, load_access_fault, status, 0, 1);
		}
	}
};

#endif // VMMEM_H
