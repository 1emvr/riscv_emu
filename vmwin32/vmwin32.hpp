#ifndef _VMWIN32_H
#define _VMWIN32_H
#include <stdint.h>

#ifdef RVM64_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif

// ntdll exports (example set)
void* NtAllocateVirtualMemory(void* ProcessHandle, void** BaseAddress, uint64_t ZeroBits, uint64_t* RegionSize, uint32_t AllocationType, uint32_t Protect);
long NtFreeVirtualMemory(void* ProcessHandle, void** BaseAddress, uint64_t* RegionSize, uint32_t FreeType);

// ucrtbase exports
//int printf(const char* fmt, ...);
//void* malloc(uint64_t size);
//void free(void* ptr);
//int strlen(const char* s);

// optionally mark as weak if you want fallback stubs or detection
__attribute__((weak)) void* GetStdHandle(int handle);

#ifdef __cplusplus
}
#endif
#endif
#endif // _VMWIN32_H
