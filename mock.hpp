#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "vmmain.hpp"
#include "vmelf.hpp"

bool read_program_from_packet() {
	HANDLE hfile = ctx->win32.NtCreateFile("./test.o", 
			GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hfile == INVALID_HANDLE_VALUE) {
		printf("ERR: failed to open file\n");
		return false;
	}

	DWORD status = ctx->win32.NtGetFileSize(hfile, &vmcs->program.size);

	if (status == INVALID_FILE_SIZE || size == 0) {
		printf("ERR: invalid or empty file\n");
		return false;
	}

	void* elf_data = nullptr;
	if (!NT_SUCCESS(ctx->win32.NtAllocateVirtualMemory(
					NtCurrentProcess(), (LPVOID)&vmcs->program.address, 0, 
					&vmcs->program.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {

		printf("ERR: failed to allocate memory for ELF\n");
		return false;
	}

	DWORD bytesRead = 0;
	if (!ctx->win32.NtReadFile(hfile, vmcs->program.address, vmcs->program.size, &bytesRead, NULL) || 
			bytesRead != size) {

		printf("ERR: failed to read file\n");
		ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID)&vmcs->program.address, MEM_RELEASE);
		return false;
	}

	bool success = rvm64::elf::load_elf64_image();
	// TODO: patch .got/.plt

	// Free the temporary ELF file buffer
	ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), &elf_data, (PSIZE_T)&size, MEM_RELEASE);

	vmcs->program.address = 0;
	vmcs->program.size = 0;

	return success;
}
#endif // MOCK_H
