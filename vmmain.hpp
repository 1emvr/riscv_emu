#ifndef VMCS_H
#define VMCS_H
#include <windows.h>
#include <cstdint>

#include "vmcommon.hpp"
#include "vmmem.hpp"

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
	uint8_t *address;
	size_t size;
} vm_buffer;


typedef struct {
	vm_range_t plt;
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
	uint32_t reason;
} vmcs_t;


__data hexane *ctx;
__data vmcs_t *vmcs;
__data HANDLE vmcs_mutex;

__data uintptr_t __stack_cookie = 0;
__rdata const uintptr_t __key = 0;


namespace rvm64::entry {
	__native void vm_init() {
		rvm64::memory::context_init();
		rvm64::rvni::resolve_ucrt_imports(); 

		while (!vmcs->halt) {
			if (!memory::read_program_from_packet()) { 
				continue;
			}
		}
	}

	__native void vm_end() {
		rvm64::memory::memory_end();
	}

	__vmcall void vm_entry() {
		while (!vmcs->halt) {
			rvm64::decoder::vm_decode(*(int32_t*)vmcs->pc);

			if (vmcs->csr.m_cause == environment_call_native) {
				rvni::vm_trap_exit();  
				continue;
			}
			if (vmcs->step) {
				vmcs->pc += 4; 
			}
		}
	}
};

#endif //VMCS_H
