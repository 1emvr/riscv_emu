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
} vm_buffer_t;


typedef struct {
	uintptr_t address;
	uintptr_t base_vaddr;
	uintptr_t entry;
	size_t size;
	vm_range_t plt;
} vm_process_t;

struct intel_t {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;

    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rflags;
};

typedef struct {
    uintptr_t pc;
    uintptr_t handler;
    uintptr_t dkey;

    uintptr_t vscratch[32];
    uintptr_t vregs[32];
    uintptr_t vstack[VSTACK_MAX_CAPACITY];

    intel_t host_context;
    intel_t vm_context;

    vm_process_t data;
    vm_process_t process;

	volatile uintptr_t trap_handler;
    volatile uintptr_t load_rsv_addr;
    volatile int load_rsv_valid;

	struct {
		uintptr_t m_epc;
		uintptr_t m_cause;
		uintptr_t m_status;
		uintptr_t m_tval;
	} csr;

    int halt;
	int trap;
} vmcs_t;


_data hexane *ctx;
_data vmcs_t *vmcs;
_data HANDLE vmcs_mutex;

_data uintptr_t stack_cookie = 0;
_rdata const uintptr_t key = 0;
_rdata const int memcheck = 1;

#endif //VMCS_H
