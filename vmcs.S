.intel_syntax noprefix

.extern save_host_context
.extern restore_host_context
.extern vm_init_memory
.extern vm_decode

.extern m_program
.extern m_program_size
.extern m_load_rsv_addr
.extern m_load_rsv_valid

.extern m_halt 
.extern m_reason
.extern m_icount
.extern m_step

.extern vm_context
.extern host_context

#ifdef __x86_64__
.set PTR_SIZE 8                          
#else
.set PTR_SIZE 4                          
#endif

.set REG_FRAME   rbp
.set REG_STACK   rsp
.set REG_ARG0    rcx
.set REG_ARG1    rdx
.set REG_ARG2    r8
.set REG_ARG3    r9
.set REG_RETVAL  rax

.set REG_VIP     rsi
.set REG_VSP     rdi
.set REG_VREG    rbx
.set REG_DKEY    r10
.set REG_SKR1    r11
.set REG_SKR2    r12
.set REG_OPND    r13
.set REG_HNDLR   r14
.set REG_PROG    r15 ;; unused

.set REGENUM_ZR  0
.set REGENUM_RA  1
.set REGENUM_SP  2 
.set REGENUM_GP  3 
.set REGENUM_TP  4
.set REGENUM_T0  5
.set REGENUM_T1  6
.set REGENUM_T2  7
.set REGENUM_S0  8
.set REGENUM_S1  9
.set REGENUM_A0  10
.set REGENUM_A1  11
.set REGENUM_A2  12 
.set REGENUM_A3  13
.set REGENUM_A4  14
.set REGENUM_A5  15
.set REGENUM_A6  16
.set REGENUM_A7  17  
.set REGENUM_S2  18
.set REGENUM_S3  19
.set REGENUM_S4  20
.set REGENUM_S5  21
.set REGENUM_S6  22
.set REGENUM_S7  23
.set REGENUM_S8  24
.set REGENUM_S9  25
.set REGENUM_S10 26
.set REGENUM_S11 27  
.set REGENUM_T3  28
.set REGENUM_T4  29
.set REGENUM_T5  30
.set REGENUM_T6  31  

.set VM_NATIVE_STACK_ALLOC             0x210
.set PROCESS_MAX_CAPACITY              (1024 * 256) 
.set VSTACK_MAX_CAPACITY               ((1024 * 16) + (16) * PTR_SIZE)  
.set VSCRATCH_MAX_CAPACITY             (64 * PTR_SIZE)
.set VREGS_MAX_CAPACITY                (32 * PTR_SIZE)

.set VREGS_SIZE                        (VREGS_MAX_CAPACITY * PTR_SIZE)
.set VSCRATCH_SIZE                     (VSCRATCH_MAX_CAPACITY * PTR_SIZE)

.set VMCS_OFFSET_MOD_BASE              (PTR_SIZE * 0)
.set VMCS_OFFSET_DKEY                  (PTR_SIZE * 1)
.set VMCS_OFFSET_PROGRAM               (PTR_SIZE * 2)
.set VMCS_OFFSET_PROGRAM_SIZE          (PTR_SIZE * 3)
.set VMCS_OFFSET_VSTACK                (PTR_SIZE * 4)
.set VMCS_OFFSET_VSCRATCH              (PTR_SIZE * 4 + VSTACK_MAX_CAPACITY)
.set VMCS_OFFSET_VREGS                 (PTR_SIZE * 4 + VSTACK_MAX_CAPACITY + VSCRATCH_MAX_CAPACITY)
	
;; NOTE: C++ functions are going to manipulate the stack pointer...
.global vm_init:	
	push  rdx

    call  vm_init_memory                                     ;; Initialize the process virtual address space.
    cmp   dword ptr [m_halt], 1                              ;; Check for memory init failure.
    jz    init_end

	mov   REG_DKEY,  __dkey                                  ;; Move the decryption key.
	mov   REG_HNDL,  __handler                               ;; Move the opcode dispatch table.
	push  REG_HNDL                                           ;; Save the opcode dispatch table address to the stack.

	mov   REG_VREGS,  [rcx + VMCS_OFFSET_VREGS]              ;; Move the address to virtual registers. 
    mov   REG_VSTACK, [rcx + VMCS_OFFSET_VSTACK]             ;; Move the address to virtual stack.

    mov   rdx,  VSTACK_MAX_CAPACITY                           ;; Calculate offset to top of the stack.
    sub   rdx,  PTR_SIZE 
    add   rdx,  REG_VSTACK                                         
    mov   [REG_VREGS + PTR_SIZE * REGENUM_SP], rdx            ;; Set the virtual register (sp) to the calculated top-of-stack.

init_end:
	pop rdx
	ret

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
