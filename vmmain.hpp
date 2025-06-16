#ifndef VMCS_H
#define VMCS_H
#include <windows.h>
#include <cstdint>

#include "vmcommon.hpp"

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

_data hexane *ctx;
_data vmcs_t *vmcs;
_data HANDLE vmcs_mutex;

_data uintptr_t stack_cookie = 0;
_rdata const uintptr_t key = 0;
_rdata const int memcheck = 1;

#endif //VMCS_H
