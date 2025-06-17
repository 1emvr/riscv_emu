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

#define csr_set(epc, cause, stat, val, hlt)	\
	vmcs->csr.m_epc = (uintptr_t)epc;		\
	vmcs->csr.m_cause = cause;				\
	vmcs->csr.m_status = stat;				\
	vmcs->csr.m_tval = val;					\
	vmcs->halt = hlt

#ifdef DEBUG
#define csr_get()						\
	uintptr_t csr1 = vmcs->csr.m_epc;	\
	uintptr_t csr2 = vmcs->csr.m_cause; \
	uintptr_t csr3 = vmcs->csr.m_status;\
	uintptr_t csr4 = vmcs->csr.m_tval;	\
	__debugbreak()
#else
#define csr_get()
#endif

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
	undefined_error = 0xFFFF,
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
