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

	/*
		// NOTE: process loading
		if (!rvm64::memory::memory_init(vmcs->data.size + VM_PROCESS_PADDING) ||
				!rvm64::elf::load_elf64_image((void*)vmcs->data.address, vmcs->data.size) ||
				!rvm64::elf::patch_elf64_imports((void*)vmcs->process.address, &vmcs->vm_plt)) {

			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			goto defer;
		}
	 */

	_native bool memory_init(size_t process_size) {
		vmcs->process.size = process_size;

		if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(
						NtCurrentProcess(), (LPVOID*) &vmcs->process.address, 0, 
						&vmcs->process.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) 
		{
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
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->host_context))) {
            vmcs->halt = 1;
        }
    }

    _native void restore_host_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->host_context))) {
            vmcs->halt = 1;
        }
    }

    _native void save_vm_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
            vmcs->halt = 1;
        }
    }

    _native void restore_vm_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
            vmcs->halt = 1;
        }
    }
};

// TODO: consider making this a macro and then just -DDEBUG when necessary. The size-cost is pretty bad, but it's more robust/safe this way, so pick-and-choose.
namespace rvm64::check {
	template <typename T>
		_vmcall bool mem_read_check(uintptr_t addr) {
			if ((addr) % sizeof(T) != 0) {                                            
				vmcs->csr.m_cause = load_address_misaligned;                          
				vmcs->csr.m_epc = vmcs->pc;                                           
				vmcs->csr.m_tval = (addr);                                            
				return false;                                                               
			}                                                                         
			if ((addr) < vmcs->process.address ||                                     
					(addr) >= vmcs->process.address + vmcs->process.size) {              

				vmcs->csr.m_cause = load_access_fault;                                
				vmcs->csr.m_epc = vmcs->pc;                                           
				vmcs->csr.m_tval = (addr);                                            
				return false;                                                               
			}                                                                         
			return true;
		}

	template <typename T>
		_vmcall bool mem_write_check(uintptr_t addr) {
			if ((addr) % sizeof(T) != 0) {                                            
				vmcs->csr.m_cause = store_amo_address_misaligned;                     
				vmcs->csr.m_epc = vmcs->pc;                                           
				vmcs->csr.m_tval = (addr);                                            
				return false;                                                               
			}                                                                         
			if ((addr) < vmcs->process.address ||                                     
					(addr) >= vmcs->process.address + vmcs->process.size) {              

				vmcs->csr.m_cause = store_amo_access_fault;                           
				vmcs->csr.m_epc = vmcs->pc;                                           
				vmcs->csr.m_tval = (addr);                                            
				return false;                                                               
			}                                                                         
			return true;
		}

	_vmcall bool reg_read_check(int reg_idx) {
		if ((reg_idx) > regenum::t6) {                                                
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (reg_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	_vmcall bool reg_write_check(int reg_idx) {
		if ((reg_idx) == regenum::zr || (reg_idx) > regenum::t6) {                        
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (reg_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	_vmcall bool scr_read_check(int scr_idx) {
		if ((scr_idx) > screnum::imm) {                                               
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (scr_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	_vmcall bool scr_write_check(int scr_idx) {
		if ((scr_idx) > screnum::imm) {                                               
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (scr_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	_vmcall bool opcall_check(int hdl_idx) {
		if ((hdl_idx) >= 255) {                                             
			vmcs->csr.m_cause = illegal_instruction;                              
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (hdl_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}
};

#endif // VMMEM_H
