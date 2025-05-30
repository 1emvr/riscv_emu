#ifndef _VMWIN32_H
#define _VMWIN32_H
#include <stdint.h>

#ifdef RVM64_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif

	void* NtAllocateVirtualMemory(void* ProcessHandle, void** BaseAddress, uint64_t ZeroBits, uint64_t* RegionSize, uint32_t AllocationType, uint32_t Protect);
	long NtFreeVirtualMemory(void* ProcessHandle, void** BaseAddress, uint64_t* RegionSize, uint32_t FreeType);

#ifdef __cplusplus
}
#endif

#endif
#endif // _VMWIN32_H
