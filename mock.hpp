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
		delctype(GetFileSize) NtGetFileSize;
		decltype(CreateFile) NtCreateFile;
		decltype(ReadFile) NtReadFile;
	} win32;
} _hexane;

bool read_program_from_packet(uintptr_t pointer) {
	HEXANE;
	
	HANDLE hfile = ctx->win32.CreateFile("./test.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD size = 0;
	DWORD status = ctx->win32.NtGetFileSize(hfile, &size);
	if (status == INVALID_FILE_SIZE) {
		return false;
	}

	return ctx->win32.NtReadFile(hfile, (void*)&pointer, size, &status, NULL);
	// TODO: file mapping likely needs to be done here.
}
