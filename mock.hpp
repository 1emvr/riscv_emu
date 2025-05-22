#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "vmmain.hpp"
#include "vmelf.hpp"

bool read_program_from_packet() {
	HANDLE hfile = ctx->win32.NtCreateFile("./test.o", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		printf("Failed to open file\n");
		return false;
	}

	DWORD size = 0;
	DWORD status = ctx->win32.NtGetFileSize(hfile, &size);
	if (status == INVALID_FILE_SIZE || size == 0) {
		printf("Invalid or empty file\n");
		return false;
	}

	void* elf_data = nullptr;
	if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(NtCurrentProcess(), &elf_data, 0, (PSIZE_T)&size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		printf("Failed to allocate memory for ELF\n");
		return false;
	}

	DWORD bytesRead = 0;
	if (!ctx->win32.NtReadFile(hfile, elf_data, size, &bytesRead, NULL) || bytesRead != size) {
		printf("Failed to read file\n");
		free(elf_data);
		return false;
	}

	vmcs->program.address = (uintptr_t)elf_data;
	vmcs->program.size = size;

	// Load (relocate) ELF from elf_data into vmcs->process.address
	bool success = rvm64::elf::load_elf64_image();

	// Free the temporary ELF file buffer
	ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), &elf_data, (PSIZE_T)&size, MEM_RELEASE);
	vmcs->program.address = 0;
	vmcs->program.size = 0;

	return success;
}
#endif // MOCK_H
