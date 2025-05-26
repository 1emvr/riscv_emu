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

#ifdef __x86_64__
#define PTR_SIZE 8
#else
#define PTR_SIZE 4
#endif

#define __extern   extern "C"
#define __function //__attribute__((section(".text$B")))
#define __rdata    __attribute__((section(".rdata")))
#define __data     __attribute__((section(".data")))
#define __used     __attribute__((used))

#define NT_SUCCESS(status)      ((status) >= 0)
#define NtCurrentProcess()      ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()       ((HANDLE) (LONG_PTR) -2)

#define VM_NATIVE_STACK_ALLOC   0x210
#define PROCESS_MAX_CAPACITY    (1024 * 256)
#define VSTACK_MAX_CAPACITY     (1024 * 1024)

#define EXPONENT_MASK           0x7FF0000000000000ULL
#define FRACTION_MASK           0x000FFFFFFFFFFFFFULL


typedef struct {
    uintptr_t address;
    size_t size;
} vm_memory_t;

typedef struct {
    CONTEXT host_context;
    CONTEXT vm_context;

    vm_memory_t program;
    vm_memory_t process;
    uint32_t pc;

    uintptr_t handler;
    uintptr_t dkey;

    uintptr_t vscratch[32];
    uintptr_t vregs[32];
    uintptr_t vstack[VSTACK_MAX_CAPACITY];

    volatile uintptr_t load_rsv_addr;
    volatile int load_rsv_valid;

    uint32_t halt;
    uint32_t reason;
    uint32_t step;
} vmcs_t;

__data hexane *ctx = nullptr;
__data vmcs_t *vmcs = nullptr;
__data HANDLE vmcs_mutex = 0;

__data uintptr_t __stack_cookie = { };
__rdata const uintptr_t __key = 0;

namespace rvm64 {
    __function void vm_entry();
    __function int64_t vm_main();
};

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
		if (src > skrenum::imm) {													\
			vmcs->halt = 1;															\
			vmcs->reason = vm_access_violation;										\
			return;                                         						\
		}																			\
		dst = (T)vmcs->vscratch[(src)];												\
	} while (0)

#define scr_write(T, dst, src)														\
	do {																			\
		if (dst > skrenum::imm) {													\
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

struct thunk {
	void* target; // The real Windows function
	uint64_t (*wrapper)(void* fn, vm_registers_t* regs); // Always generic_thunk
};

std::unordered_map<void*, thunk*> thunk_table;

typedef uint64_t (*win_func_t)(...);
uint64_t call_windows_function(void* func, vm_registers_t* regs) {
    win_func_t winfn = (win_func_t)func;

    uint64_t a0 = regs->x[10];
    uint64_t a1 = regs->x[11];
    uint64_t a2 = regs->x[12];
    uint64_t a3 = regs->x[13];
    uint64_t a4 = regs->x[14];
    uint64_t a5 = regs->x[15];
    uint64_t a6 = regs->x[16];
    uint64_t a7 = regs->x[17];

    return winfn(a0, a1, a2, a3, a4, a5, a6, a7);
}

uint64_t generic_thunk(void* fn, vm_registers_t* regs) {
	return call_windows_function(fn, regs);
}

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

enum skrenum {
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
	rtype = 1, r4type, itype, stype, btype, utype, jtype, ftype,
};

#endif //VMCS_H
