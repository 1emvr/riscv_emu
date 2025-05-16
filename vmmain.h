#ifndef VMCS_H
#define VMCS_H

#include "mono.hpp"

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
    uintptr_t mod_base;
    uintptr_t handler;
    uintptr_t dkey;
    uintptr_t pc;

    vm_memory_t program;
    vm_memory_t process;

    uint8_t vstack[VSTACK_MAX_CAPACITY];
    uint8_t vregs[VREGS_MAX_CAPACITY];

    uint8_t vscratch1[8];
    int32_t vscratch2[8];

    CONTEXT host_context;
    CONTEXT vm_context;

    uintptr_t load_rsv_addr;
    int load_rsv_valid;

    int halt;
    int reason;
    int step;
} vmcs_t;


__data hexane *ctx = nullptr;
__data vmcs_t *vmcs = nullptr;
__data uintptr_t __stack_cookie = { };
__data uintptr_t __key;
#endif //VMCS_H
