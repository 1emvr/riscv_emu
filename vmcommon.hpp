#ifndef _VMCOMMON_H
#define _VMCOMMON_H
#include <windows.h>

typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSTATUS(NTAPI* NtFreeVirtualMemory_t)(HANDLE processHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType);
typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef NTSTATUS(NTAPI* NtSetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef PVOID(NTAPI* RtlAllocateHeap_t)(HANDLE HeapHandle, ULONG Flags, SIZE_T Size);

#define _native //__attribute__((section(".text$B")))
#define _vmcall //__attribute__((section(".text$B"))) __attribute__((calling_convention("custom")))
#define _naked  //__declspec(naked)

#define _rdata    __attribute__((section(".rdata")))
#define _data     __attribute__((section(".data")))
#define _extern   extern "C"

#define NT_SUCCESS(status)      ((status) >= 0)
#define NtCurrentProcess()      ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()       ((HANDLE) (LONG_PTR) -2)

#define VM_NATIVE_STACK_ALLOC   0x210
#define VM_PROCESS_PADDING      (1024 * 256)
#define VSTACK_MAX_CAPACITY     (1024 * 1024)

#define EXPONENT_MASK           0x7FF0000000000000ULL
#define FRACTION_MASK           0x000FFFFFFFFFFFFFULL

#define CSR_SET_TRAP(epc, cause, stat, val, hlt)	\
	vmcs->csr.m_epc = (uintptr_t)epc;				\
	vmcs->csr.m_cause = cause;						\
	vmcs->csr.m_status = stat;						\
	vmcs->csr.m_tval = val;							\
	vmcs->halt = hlt;								\
	__debugbreak()

#ifdef DEBUG
#define CSR_GET(ctx_ptr)							\
	do {											\
		uintptr_t csr1 = vmcs->csr.m_epc;			\
		uintptr_t csr2 = vmcs->csr.m_cause; 		\
		uintptr_t csr3 = vmcs->csr.m_status;		\
		uintptr_t csr4 = vmcs->csr.m_tval;			\
		uintptr_t ip = ctx_ptr->ContextRecord->Rip; \
		__debugbreak();								\
	} while (0)
#else
#define CSR_GET(ctx_ptr) {}
#endif

#define SAVE_VM_CONTEXT(expr)	\
	save_vm_context();			\
	expr;						\
	restore_vm_context()

enum causenum {
	supervisor_software_interrupt = 0xb11,
	machine_software_interrupt = 0xb13,
	supervisor_timer_interrupt = 0xb15,
	machine_timer_interrupt = 0xb17,
	supervisor_external_interrupt = 0xb19,
	machine_external_interrupt = 0xb111,
	designated_for_platform_use = 0xb116,
	instruction_address_misaligned = 0xb00,
	instruction_access_fault = 0xb01,
	illegal_instruction = 0xb02,
	breakpoint = 0xb03,
	load_address_misaligned = 0xb04,
	load_access_fault = 0xb05,
	store_amo_address_misaligned = 0xb06,
	store_amo_access_fault = 0xb07,
	environment_call_from_u_mode = 0xb08,
	environment_call_from_s_mode = 0xb09,
	environment_call_from_m_mode = 0xb011,
	instruction_page_fault = 0xb012,
	load_page_fault = 0xb013,
	store_amo_page_fault = 0xb015,
	environment_call_native = 0xb024,
	image_bad_symbol = 0xb025,
	image_bad_load = 0xb026,
	image_bad_type = 0xb027,
	designated_for_custom_use_5 = 0xb028,
	designated_for_custom_use_6 = 0xb029,
	designated_for_custom_use_7 = 0xb030,
	designated_for_custom_use_8 = 0xb031,
	undefined = 0xffff,
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
#endif // _VMCOMMON_H
