#ifndef VMCS_H
#define VMCS_H

#include "monodef.hpp"

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
    uintptr_t address;
    size_t size;
} vm_memory_t;

typedef struct {
    CONTEXT host_context;
    CONTEXT vm_context;

    vm_memory_t program;
    vm_memory_t process;

    uintptr_t handler;
    uintptr_t dkey;
    uintptr_t pc;

    uintptr_t vscratch[32];
    uintptr_t vregs[32];
    uintptr_t vstack[VSTACK_MAX_CAPACITY];

    volatile uintptr_t load_rsv_addr;
    volatile int load_rsv_valid;

    int halt;
    int reason;
    int step;
} vmcs_t;

__data hexane *ctx = nullptr;
__data vmcs_t *vmcs = nullptr;
__data HANDLE vmcs_mutex = 0;
__data uintptr_t __stack_cookie = { };
__data uintptr_t __key = 0;

namespace rvm64 {
    __function void vm_entry();
    __function int64_t vm_main();
};
#endif //VMCS_H
