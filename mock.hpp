#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "vmmain.hpp"
#include "vmcommon.hpp"

namespace rvm64::mock {
	__native vm_buffer* read_file() {
		BOOL success = false;
		DWORD bytes_read = 0;

		vm_buffer *buffer = (vm_buffer*)malloc(sizeof(vm_buffer));

		HANDLE hfile = ctx->win32.NtCreateFile("./test.o", GENERIC_READ, FILE_SHARE_READ, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hfile == INVALID_HANDLE_VALUE) { // NOTE: since not reading from the network, this can only pass/fail - exit immediately
			vmcs->mepc = vmcs->pc;
			vmcs->reason = vm_undefined;
			vmcs->halt = 1;
			goto defer;
		}

		vmcs->reason = ctx->win32.NtGetFileSize(hfile, (LPDWORD)&buffer->size);

		if (vmcs->reason == INVALID_FILE_SIZE || buffer->size == 0) {
			vmcs->halt = 1;
			goto defer;
		}

		if (!ctx->win32.NtReadFile(hfile, (LPVOID)buffer->address, buffer->size, &bytes_read, NULL) || 
				bytes_read != buffer->size) {

			vmcs->halt = 1;
			vmcs->reason = vm_undefined;
			goto defer;
		}

		success = true;
defer:
		if (hfile) {
			CloseHandle((HANDLE)hfile);
		}
		if (!success) {
			free(buffer);
			buffer = nullptr;
		}

		return buffer;
	}

	__native bool read_program_from_packet() {
		vm_buffer *data = rvm64::mock::read_file();
		if (!data) {
			return false;
		}

		data->size += VM_PROCESS_PADDING;

		// TODO: get plt start/end addresses
		rvm64::memory::memory_init(data->size);
		rvm64::elf::load_elf_image(data->address, data->size);
		rvm64::elf::patch_elf_imports();

		if (data) {
			free((void*)data->address);
			free((void*)data);
		}
		return true;
	}
};
#endif // MOCK_H
