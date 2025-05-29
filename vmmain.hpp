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

	struct {
		uintptr_t m_epc;
		uintptr_t m_cause;
		uintptr_t m_status;
		uintptr_t m_tval;
	} csr;

    uint32_t halt;
    uint32_t step;
} vmcs_t;

#define __native //__attribute__((section(".text$B")))
#define __vmcall //__attribute__((section(".text$B"))) __attribute__((calling_convention("custom")))

#define __rdata    	__attribute__((section(".rdata")))
#define __data     	__attribute__((section(".data")))
#define __extern   	extern "C"

#define NT_SUCCESS(status)      ((status) >= 0)
#define NtCurrentProcess()      ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()       ((HANDLE) (LONG_PTR) -2)

#define VM_NATIVE_STACK_ALLOC   0x210
#define VM_PROCESS_PADDING      (1024 * 256)
#define VSTACK_MAX_CAPACITY     (1024 * 1024)

#define EXPONENT_MASK           0x7FF0000000000000ULL
#define FRACTION_MASK           0x000FFFFFFFFFFFFFULL

// TODO: move these to rvm64::memory (?)
// NOTE: this could be a disaster for performance...

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

__vmcall bool opcode_check(int hdl_idx) {
	if ((hdl_idx) >= sizeof(__handler) / sizeof(__handler[0])) {                                             
		vmcs->csr.m_cause = illegal_instruction;                              
		vmcs->csr.m_epc = vmcs->pc;                                           
		vmcs->csr.m_tval = (hdl_idx);                                             
		return false;                                                               
	}                                                                         
	return true;
}

#define mem_read(T, retval, addr)  													\
	do {																			\
		if (!mem_read_check<T>(addr)) { 											\
			vmcs->halt = 1; 														\
			return; 																\
		} 																			\
		retval = *(T *)(vmcs->process.address +  									\
				((addr) - vmcs->process.address)); 									\
	} while(0)

#define mem_write(T, addr, value)  													\
	do {																			\
		if (!mem_write_check<T>(addr)) { 											\
			vmcs->halt = 1; 														\
			return; 																\
		} 																			\
		*(T *)(vmcs->process.address +  											\
				((addr) - vmcs->process.address)) = value;  						\
	} while(0)

#define reg_read(T, dst, reg_idx) 													\
	do { 																			\
		if (!reg_read_check(reg_idx)) { 											\
			vmcs->halt = 1; 														\
			return; 																\
		} 																			\
		dst = (T)vmcs->vregs[(reg_idx)];                                            \
	} while(0)

#define reg_write(T, reg_idx, src) 													\
	do { 																			\
		if (!reg_write_check(reg_idx)) { 											\
			vmcs->halt = 1; 														\
			return; 																\
		} 																			\
		vmcs->vregs[(reg_idx)] = (T)(src);                                          \
	} while(0)

#define scr_read(T, dst, scr_idx) 													\
	do { 																			\
		if (!scr_read_check(scr_idx)) { 											\
			vmcs->halt = 1; 														\
			return; 																\
		} 																			\
		dst = (T)vmcs->vscratch[(scr_idx)];                                         \
	} while(0)

#define scr_write(T, scr_idx, src) 													\
	do { 																			\
		if (!scr_write_check(scr_idx)) { 											\
			vmcs->halt = 1; 														\
			return; 																\
		} 																			\
		vmcs->vscratch[(scr_idx)] = (T)(src);                                 		\
	} while(0)

#define unwrap_opcall(hdl_idx) 														\
	do { 																			\
		if (!opcall_check(hdl_idx)) { 												\
			vmcs->halt = 1; 														\
			return; 																\
		} 																			\
		auto a = ((uintptr_t*)vmcs->handler)[hdl_idx];                              \
		auto b = rvm64::crypt::decrypt_ptr((uintptr_t)a);                         	\
		void (*fn)() = (void (*)())(b);                                           	\
		fn();                                                                     	\
	} while(0)

__data hexane *ctx;
__data vmcs_t *vmcs;
__data HANDLE vmcs_mutex;

__data uintptr_t __stack_cookie = 0;
__rdata const uintptr_t __key = 0;

namespace rvm64 {
    __vmcall void vm_entry();
    __native int64_t vm_main();
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
