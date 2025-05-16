#ifndef VMCS_H
#define VMCS_H

#include "mono.hpp"
#include "mock.hpp"

typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSTATUS(NTAPI* NtFreeVirtualMemory_t)(HANDLE processHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType);
typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef NTSTATUS(NTAPI* NtSetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);

typedef struct {
    uintptr_t mod_base;
    uintptr_t dkey;
    uintptr_t handler;
    uintptr_t program;
    uintptr_t program_size;

    uint8_t vstack[VSTACK_MAX_CAPACITY];
    uint8_t vregs[VREGS_MAX_CAPACITY];
    uint8_t vscratch[VSCRATCH_MAX_CAPACITY];

    CONTEXT host_context;
    CONTEXT vm_context;

    uintptr_t load_rsv_addr;
    int load_rsv_valid;

    int halt;
    int reason;
    int step;
} vmcs_t;

typedef struct __hexane {
    struct {
        NtGetContextThread_t NtGetContextThread;
        NtSetContextThread_t NtSetContextThread;
        NtAllocateVirtualMemory_t NtAllocateVirtualMemory;
        NtFreeVirtualMemory_t NtFreeVirtualMemory;

        decltype(CreateFileA)* NtCreateFile;
        decltype(GetFileSize)* NtGetFileSize;
        decltype(ReadFile)* NtReadFile;
        decltype(OpenFile)* NtOpenFile;
    } win32;
} hexane;

extern hexane *ctx;
extern vmcs_t *vmcs;
extern uintptr_t __stack_cookie;
extern uintptr_t __key;
#endif //VMCS_H
