#ifndef _VMCOMMON_H
#define _VMCOMMON_H
#include <windows.h>
#include "vmcrypt.hpp"


typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSTATUS(NTAPI* NtFreeVirtualMemory_t)(HANDLE processHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType);
typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef NTSTATUS(NTAPI* NtSetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef PVOID(NTAPI* RtlAllocateHeap_t)(HANDLE HeapHandle, ULONG Flags, SIZE_T Size);

#define _native //__attribute__((section(".text$B")))
#define _vmcall //__attribute__((section(".text$B"))) __attribute__((calling_convention("custom")))

#define _rdata    	__attribute__((section(".rdata")))
#define _data     	__attribute__((section(".data")))
#define _extern   	extern "C"

#define NT_SUCCESS(status)      ((status) >= 0)
#define NtCurrentProcess()      ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()       ((HANDLE) (LONG_PTR) -2)

#define VM_NATIVE_STACK_ALLOC   0x210
#define VM_PROCESS_PADDING      (1024 * 256)
#define VSTACK_MAX_CAPACITY     (1024 * 1024)

#define EXPONENT_MASK           0x7FF0000000000000ULL
#define FRACTION_MASK           0x000FFFFFFFFFFFFFULL


enum causenum {
	instruction_address_misaligned = 0xF00,
	instruction_access_fault = 0xF01,
	illegal_instruction = 0xF02,
	breakpoint = 0xF03,
	load_address_misaligned = 0xF04,
	load_access_fault = 0xF05,
	store_amo_address_misaligned = 0xF06,
	store_amo_access_fault = 0xF07,
	environment_call_u_mode = 0xF08,
	environment_call_s_mode = 0xF09,
	supervisor_software_interrupt = 0xF11,
	machine_software_interrupt = 0xF13,
	supervisor_timer_interrupt = 0xF15,
	machine_timer_interrupt = 0xF17,
	supervisor_external_interrupt = 0xF19,
	environment_call_m_mode = 0xF011,
	instruction_page_fault = 0xF012,
	load_page_fault = 0xF013,
	environment_call_native = 0xF014,
	store_amo_page_fault = 0xF015,
	machine_external_interrupt = 0xF111,
};

enum vm_reason {
	vm_ok,
	vm_illegal_op,
	vm_access_violation,
	vm_unaligned_op,
	vm_invalid_pc,
	vm_return_address_corruption,
	vm_stack_overflow,
	vm_undefined,
	vm_user_ecall,
	vm_debug_break,
};

enum screnum {
    rd, rs1, rs2, rs3, imm,
};

enum regenum {
	zr, ra, sp, gp, tp,
	t0, t1, t2, s0, s1,
	a0, a1, a2, a3, a4, a5, a6, a7,
	s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
	t3, t4, t5, t6,
};

enum typenum {
	rtype = 1, r4type, itype, stype, btype, utype, jtype,
};

#define mem_read(T, retval, addr)  								\
	do {														\
		retval = *(T *)(vmcs->process.address +  				\
				((addr) - vmcs->process.address)); 				\
	} while(0)

#define mem_write(T, addr, value)  								\
	do {														\
		*(T *)(vmcs->process.address +  						\
				((addr) - vmcs->process.address)) = value;  	\
	} while(0)

#define reg_read(T, dst, reg_idx) 								\
	do { 														\
		dst = (T)vmcs->vregs[(reg_idx)];						\
	} while(0)

#define reg_write(T, reg_idx, src) 								\
	do { 														\
		vmcs->vregs[(reg_idx)] = (T)(src);						\
	} while(0)

#define scr_read(T, dst, scr_idx) 								\
	do { 														\
		dst = (T)vmcs->vscratch[(scr_idx)];						\
	} while(0)

#define scr_write(T, scr_idx, src) 								\
	do { 														\
		vmcs->vscratch[(scr_idx)] = (T)(src);					\
	} while(0)

#endif // _VMCOMMON_H
