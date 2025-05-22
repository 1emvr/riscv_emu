#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "vmmain.hpp"
#include "vmelf.hpp"

bool read_program_from_packet() {
	NTSTATUS status = 0;
	HANDLE hfile = ctx->win32.NtCreateFile("./test.o", 
			GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE) {
		vmcs->halt = 1;
		vmcs->reason = status;
		return false;
	}

	status = ctx->win32.NtGetFileSize(hfile, &vmcs->program.size);

	if (status == INVALID_FILE_SIZE || size == 0) {
		vmcs->halt = 1;
		vmcs->reason = status;
		return false;
	}

	void* elf_data = nullptr;
	if (!NT_SUCCESS(status = ctx->win32.NtAllocateVirtualMemory(
					NtCurrentProcess(), (LPVOID)&vmcs->program.address, 0, 
					&vmcs->program.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {

		vmcs->halt = 1;
		vmcs->reason = status;
		return false;
	}

	DWORD bytesRead = 0;
	if (!ctx->win32.NtReadFile(hfile, vmcs->program.address, vmcs->program.size, &bytesRead, NULL) || 
			bytesRead != size) {

		ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID)&vmcs->program.address, MEM_RELEASE);
		vmcs->halt = 1;
		vmcs->reason = vm_invalid_value;
		return false;
	}

	bool success = rvm64::elf::load_elf64_image();
	// TODO: patch .got/.plt

	// Free the temporary ELF file buffer
	ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID)&vmcs->program.address, &vmcs->program.size, MEM_RELEASE);

	vmcs->program.address = 0;
	return success;
}
#endif // MOCK_H
