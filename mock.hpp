#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "vmmain.hpp"
#include "vmelf.hpp"

namespace mock {
	_function bool read_program_from_packet() {
		BOOL success = false;
		DWORD bytes_read = 0;

		HANDLE hfile = ctx->win32.NtCreateFile("./test.o", GENERIC_READ, FILE_SHARE_READ, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hfile == INVALID_HANDLE_VALUE) { // NOTE: since not reading from the netowrk, this can only pass/fail - exit immediately
			vmcs->reason = vm_undefined;
			vmcs->halt = 1;
			goto defer;
		}

		vmcs->reason = ctx->win32.NtGetFileSize(hfile, (LPDWORD) &vmcs->data.size);

		if (vmcs->reason == INVALID_FILE_SIZE || vmcs->data.size == 0) {
			vmcs->halt = 1;
			goto defer;
		}

		if (!ctx->win32.NtReadFile(hfile, (LPVOID)vmcs->data.address, vmcs->data.size, &bytes_read, NULL) || 
				bytes_read != vmcs->data.size) {

			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			goto defer;
		}

		if (!rvm64::memory::vm_mem_init(vmcs->data.size + VM_PROCESS_PADDING) ||
				!rvm64::elf::load_elf64_image((void*)vmcs->data.address, vmcs->data.size) ||
				!rvm64::elf::patch_elf64_imports((void*)vmcs->process.address)) {

			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			goto defer;
		}

defer:
		if (hfile) {
			CloseHandle((HANDLE)hfile);
		}
		if (vmcs->data.address) {
			ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (LPVOID*) &vmcs->data.address, &vmcs->data.size, MEM_RELEASE);
			vmcs->data.address = 0;
			vmcs->data.size = 0;
		}
		return success;
	}
};
#endif // MOCK_H
