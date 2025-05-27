#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "vmmain.hpp"
#include "vmelf.hpp"

_function bool read_program_from_packet() {
	BOOL success = false;
	NTSTATUS status = 0;

	HANDLE hfile = ctx->win32.NtCreateFile("./test.o", 
			GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE) {
		vmcs->reason = vm_undefined;
		vmcs->halt = 1;
		goto defer;
	}

	vmcs->reason = ctx->win32.NtGetFileSize(hfile, &vmcs->program.size);

	if (status == INVALID_FILE_SIZE || size == 0) {
		vmcs->halt = 1;
		goto defer;
	}

	if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(
					NtCurrentProcess(), (LPVOID)&vmcs->program.address, 0, 
					&vmcs->program.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		vmcs->halt = 1;
		goto defer;
	}

	DWORD bytesRead = 0;
	if (!ctx->win32.NtReadFile(hfile, vmcs->program.address, vmcs->program.size, &bytesRead, NULL) || 
			bytesRead != size) {

		vmcs->halt = 1;
		vmcs->reason = vm_undefined;
		goto defer;
	}

	rvm64::elf::load_elf64_image();
	rvm64::elf::patch_elf64_imports();

defer:
	if (vmcs->program.address) {
		ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID)&vmcs->program.address, &vmcs->program.size, MEM_RELEASE);
	}

	vmcs->program.address = 0;
	return success;
}
#endif // MOCK_H
