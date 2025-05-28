#ifndef VMCS_H
#define VMCS_H

typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSTATUS(NTAPI* NtFreeVirtualMemory_t)(HANDLE processHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType);
typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef NTSTATUS(NTAPI* NtSetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef PVOID(NTAPI* RtlAllocateHeap_t)(HANDLE HeapHandle, ULONG Flags, SIZE_T Size);

typedef struct __hexane {
    void *heap;
    struct {
        NtGetContextThread_t NtGetContextThread;
        NtSetContextThread_t NtSetContextThread;
        NtAllocateVirtualMemory_t NtAllocateVirtualMemory;
        NtFreeVirtualMemory_t NtFreeVirtualMemory;
        RtlAllocateHeap_t RtlAllocateHeap;

        decltype(WaitForSingleObject)* NtWaitForSingleObject;
        decltype(CreateMutexA)* NtCreateMutex;
        decltype(ReleaseMutex)* NtReleaseMutex;
        decltype(CreateFileA)* NtCreateFile;
        decltype(GetFileSize)* NtGetFileSize;
        decltype(ReadFile)* NtReadFile;
        decltype(OpenFile)* NtOpenFile;
    } win32;
} hexane;

typedef struct {
	uintptr_t start;
	uintptr_t end;
} vm_range_t;

typedef struct {
	vm_range_t vm_plt;
	uintptr_t address;
	uintptr_t base_vaddr;
	uintptr_t entry;
	size_t vsize;
	size_t size;
} vm_memory_t;

typedef struct {
    uintptr_t pc;
    uintptr_t handler;
    uintptr_t dkey;

    uintptr_t vscratch[32];
    uintptr_t vregs[32];
    uintptr_t vstack[VSTACK_MAX_CAPACITY];

    CONTEXT host_context;
    CONTEXT vm_context;

    vm_memory_t data;
    vm_memory_t process;

    volatile uintptr_t load_rsv_addr;
    volatile int load_rsv_valid;

    uint32_t halt;
    uint32_t reason;
    uint32_t step;
} vmcs_t;

#ifdef __x86_64__
#define PTR_SIZE 8
#else
#define PTR_SIZE 4
#endif

#define _extern   extern "C"
#define _function //__attribute__((section(".text$B")))
#define _rdata    __attribute__((section(".rdata")))
#define _data     __attribute__((section(".data")))
#define _used     __attribute__((used))

#define NT_SUCCESS(status)      ((status) >= 0)
#define NtCurrentProcess()      ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()       ((HANDLE) (LONG_PTR) -2)

#define VM_NATIVE_STACK_ALLOC   0x210
#define VM_PROCESS_PADDING      (1024 * 256)
#define VSTACK_MAX_CAPACITY     (1024 * 1024)

#define EXPONENT_MASK           0x7FF0000000000000ULL
#define FRACTION_MASK           0x000FFFFFFFFFFFFFULL

#define mem_read(T, retval, addr)                                               	\
	do {						                                                	\
		if ((addr) % sizeof(T) != 0) {												\
			vmcs->halt = 1;															\
			vmcs->reason = vm_unaligned_op;											\
			return;																	\
		}																			\
		if ((addr) < vmcs->process.address || 										\
				(addr) > vmcs->process.address + PROCESS_MAX_CAPACITY) {			\
			vmcs->halt = 1;															\
			vmcs->reason = vm_access_violation;										\
			return;																	\
		}																			\
		retval = *(T*) (vmcs->process.address + ((addr) - vmcs->process.address));	\
	} while (0)

#define mem_write(T, addr, value)                                               	\
	do {											        						\
		if ((addr) % sizeof(T) != 0) {												\
			vmcs->halt = 1;															\
			vmcs->reason = vm_unaligned_op;											\
			return;																	\
		}																			\
		if ((addr) < vmcs->process.address ||  										\
				(addr) > vmcs->process.address + PROCESS_MAX_CAPACITY) {			\
			vmcs->halt = 1;															\
			vmcs->reason = vm_access_violation;										\
			return;																	\
		}																			\
		*(T*) (vmcs->process.address + ((addr) - vmcs->process.address)) = (value); \
	} while (0)

#define reg_read(T, dst, src)														\
	do {																			\
		if ((src) > regenum::t6) {													\
			vmcs->halt = 1;															\
			vmcs->reason = vm_access_violation;										\
			return;																	\
		}																			\
		dst = (T)vmcs->vregs[(src)];												\
	} while(0)

#define reg_write(T, dst, src)														\
	do {                                                    						\
		if ((dst) == regenum::zr || (dst) > regenum::t6) {							\
			vmcs->halt = 1;															\
			vmcs->reason = vm_access_violation;										\
			return;                                         						\
		}                                                   						\
		vmcs->vregs[(dst)] = (T)src;												\
	} while (0)

#define scr_read(T, dst, src)														\
	do {																			\
		if (src > screnum::imm) {													\
			vmcs->halt = 1;															\
			vmcs->reason = vm_access_violation;										\
			return;                                         						\
		}																			\
		dst = (T)vmcs->vscratch[(src)];												\
	} while (0)

#define scr_write(T, dst, src)														\
	do {																			\
		if (dst > screnum::imm) {													\
			vmcs->halt = 1;															\
			vmcs->reason = vm_access_violation;										\
			return;                                         						\
		}																			\
		vmcs->vscratch[(dst)] = (T)src;												\
	} while (0)

#define unwrap_opcall(idx)															\
	do {																			\
		auto a = ((uintptr_t*)vmcs->handler)[idx];									\
		auto b = rvm64::crypt::decrypt_ptr((uintptr_t)a);							\
		void (*fn)() = (void (*)())(b);												\
		fn();																		\
	} while(0)

_data hexane *ctx = nullptr;
_data vmcs_t *vmcs = nullptr;
_data HANDLE vmcs_mutex = 0;

_data uintptr_t __stack_cookie = 0;
_rdata const uintptr_t __key = 0;

namespace rvm64 {
    _function void vm_entry();
    _function int64_t vm_main();
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

#endif //VMCS_H
