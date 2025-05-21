#include "monodef.hpp"
#include "vmctx.h"
#include "vmops.h"
#include "vmmain.h"

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


    /*
.global vm_entry:
	push  rbp
	mov   rbp, rsp                                           ;; save the original native stack pointer.
	sub   rsp, VM_NATIVE_STACK_ALLOC                         ;; allocate a large amount of native stack space for the vm.
	and   rsp, 0xFFFFFFFFFFFFFFF0                            ;; align the native stack pointer to 16-byte boundary.

	call  save_host_context                                  ;; save the hosts register/flags to the native stack.
	mov   REG_VIP, m_program                                 ;; prepare the vip with the program pointer to start.
	call  vm_routine                                         ;; load and call the main routine for virtual machine

	call  restore_host_context                               ;; restore original register state stored on the stack
	mov   rsp, rbp                                           ;; when finished with the routine, restore the original stack pointer.
	pop   rbp
	ret
	*/

    /*
.global vm_routine:
	cmp   dword ptr [m_halt], 1                              ;; Check for a system halt on each iteration.
	jz    routine_end                                        ;; Jump to exit on 1.

	mov   dword ptr [m_step], 1                              ;; Reset the step flag after each iteration
	mov   REG_OPND, dword ptr [REG_VIP]                      ;; Read the opcode from the virtual instruction pointer to the first argument
    xor   REG_OPND, REG_DKEY                                 ;; Decrypt opcode

    mov   rcx, REG_OPND                                      ;; Load the opcode as the first argument.
	call  vm_decode                                          ;; Decode the opcode and find it's associated function pointer.

    mov   rcx, REG_OPND                                      ;; Reload the opcode as the first argument.
	call  rax                                                ;; Call this decoded function pointer.

	cmp   dword ptr [m_step], 1                              ;; Check the step flag. During jump operations the step flag should be false.
	jnz   vm_routine                                         ;; If step is false, do not move the instruction ptr and just continue reading.

	add   REG_VIP,  4                                        ;; If step is true, move the instruction pointer forward by a word.
	jmp   vm_routine                                         ;; Continue reading.

routine_end:
	ret
	*/