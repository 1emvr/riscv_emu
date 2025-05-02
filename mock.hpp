#include <windows.h>
#define NtCurrentProcess()  ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()   ((HANDLE) (LONG_PTR) -2)

typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSTATUS(NTAPI* NtFreeVirtualMemory_t)(HANDLE processHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType);
typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);
typedef NTSTATUS(NTAPI* NtSetContextThread_t)(HANDLE ThreadHandle, PCONTEXT ThreadContext);

struct {
	struct {
		NtGetContextThread_t NtGetContextThread;
		NtSetContextThread_t NtSetContextThread;
		NtAllocateVirtualMemory_t NtAllocateVirtualMemory;
		NtFreeVirtualMemory_t NtFreeVirtualMemory;
		decltype(ReadFile) NtReadFile;
	} win32;
} _hexane;

bool read_program_from_packet(uintptr_t pointer) {
	HEXANE;
	
}
