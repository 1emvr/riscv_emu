#ifndef VMMEM_H
#define VMMEM_H
#include "vmmain.hpp"
#include "vmops.hpp"

namespace rvm64::memory {
    constexpr uintptr_t encrypt_ptr(uintptr_t ptr) {
        return ptr ^ __key;
    }

    __vmcall uintptr_t decrypt_ptr(uintptr_t ptr) {
        return ptr ^ vmcs->dkey;
    }

    __native void context_init() {
		vmcs->dkey = __key; 
		vmcs->handler = (uintptr_t)__handler;

		vmcs->load_rsv_valid = false;
		vmcs->load_rsv_addr = 0LL;

		vmcs->csr.m_cause = 0;                                
		vmcs->csr.m_epc = 0;                                           
		vmcs->csr.m_tval = 0;                                            
		vmcs->halt = 0;

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

    __vmcall void vm_set_load_rsv(int hart_id, uintptr_t address) {
        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = address; // vmcs_array[hart_id]->load_rsv_addr = address;
        vmcs->load_rsv_valid = true; // vmcs_array[hart_id]->load_rsv_valid = true;

        ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    __vmcall void vm_clear_load_rsv(int hart_id) {
        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = 0LL; // vmcs_array[hart_id]->load_rsv_addr = 0LL;
        vmcs->load_rsv_valid = false; // vmcs_array[hart_id]->load_rsv_valid = false;

        ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    __vmcall bool vm_check_load_rsv(int hart_id, uintptr_t address) {
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

	__native bool memory_init(size_t process_size) {
		vmcs->process.size = process_size;

		if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(
						NtCurrentProcess(), (LPVOID*) &vmcs->process.address, 0, 
						&vmcs->process.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) 
		{
			return false;
		}
	}

	__native void memory_end() {
		vmcs->reason = ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID*)&vmcs->process.address, &vmcs->process.size, MEM_RELEASE);
	}

	__native bool read_program_from_packet() {
		vm_buffer *data = mock::read_file();
		if (!data) {
			return false;
		}

		data->size += VM_PROCESS_PADDING;

		memory_init(data->size);
		rvm64::elf::load_elf_image(data->address, data->size);
		rvm64::elf::patch_elf_imports(data->address);
	}

	__rdata const uintptr_t __handler[256] = {
		// ITYPE
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_addi), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_slti),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_sltiu), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_xori),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ori), 			encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_andi),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_slli), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_srli),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_srai), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_addiw),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_slliw), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_srliw),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_sraiw), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lb),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lh), 			encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lw),

		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lbu), 			encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lhu),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lwu), 			encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ld),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_flq), 			encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fence),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fence_i), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_jalr),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ecall), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ebreak),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrw), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrs),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrc), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrwi),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrsi), 		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrci),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fclass_d), 	encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lrw),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lrd), 			encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fmv_d_x),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_s_d), 	encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_d_s),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_w_d), 	encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_wu_d),
		encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_d_w), 	encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_d_wu),

		// RTYPE
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fadd_d), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsub_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fmul_d), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fdiv_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsqrt_d), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsgnj_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsgnjn_d), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsgnjx_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fmin_d), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fmax_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_feq_d), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_flt_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fle_d), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_scw),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoswap_w), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoadd_w),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoxor_w), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoand_w),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoor_w), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomin_w),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomax_w), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amominu_w),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomaxu_w),

		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_scd),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoswap_d), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoadd_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoxor_d), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoand_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoor_d), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomin_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomax_d), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amominu_d),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomaxu_d),

		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_addw), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_subw),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulw), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_srlw),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sraw), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_divuw),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sllw), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_divw),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_remw), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_remuw),

		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_add), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sub),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mul), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sll),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulh), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_slt),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulhsu), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sltu),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulhu), 	encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_xor),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_div), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_srl),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sra), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_divu),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_or), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_rem),
		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_and), 		encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_remu),

		// TODO: Finish floating point instructions (probably S and I types) - f, fm, fnm, fcvt - have a few already done.
		// STYPE
		encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sb), 	encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sh),
		encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sw), 	encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sd),
		encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_fsw), 	encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_fsd),

		// BTYPE
		encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_beq), 	encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bne),
		encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_blt), 	encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bge),
		encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bltu), encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bgeu),

		// UTYPE/JTYPE
		encrypt_ptr((uintptr_t) rvm64::operations::utype::rv_lui), 	encrypt_ptr((uintptr_t) rvm64::operations::utype::rv_auipc),
		encrypt_ptr((uintptr_t) rvm64::operations::jtype::rv_jal)
	};
};

namespace rvm64::context {
    __native void save_host_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->host_context))) {
            vmcs->halt = 1;
        }
    }

    __native void restore_host_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->host_context))) {
            vmcs->halt = 1;
        }
    }

    __native void save_vm_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtGetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
            vmcs->halt = 1;
        }
    }

    __native void restore_vm_context() {
        if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtSetContextThread(NtCurrentThread(), &vmcs->vm_context))) {
            vmcs->halt = 1;
        }
    }
};

namespace rvm64::check {
	template <typename T>
		__vmcall bool mem_read_check(uintptr_t addr) {
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
		__vmcall bool mem_write_check(uintptr_t addr) {
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

	__vmcall bool reg_read_check(int reg_idx) {
		if ((reg_idx) > regenum::t6) {                                                
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (reg_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	__vmcall bool reg_write_check(int reg_idx) {
		if ((reg_idx) == regenum::zr || (reg_idx) > regenum::t6) {                        
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (reg_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	__vmcall bool scr_read_check(int scr_idx) {
		if ((scr_idx) > screnum::imm) {                                               
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (scr_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	__vmcall bool scr_write_check(int scr_idx) {
		if ((scr_idx) > screnum::imm) {                                               
			vmcs->csr.m_cause = instruction_access_fault;                         
			vmcs->csr.m_epc = vmcs->pc;                                           
			vmcs->csr.m_tval = (scr_idx);                                             
			return false;                                                               
		}                                                                         
		return true;
	}

	__vmcall bool opcall_check(int hdl_idx) {
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
