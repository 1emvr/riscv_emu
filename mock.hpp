#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstring>

#include "vmmain.hpp"
#include "vmcommon.hpp"
#include "vmmem.hpp"
#include "vmelf.hpp"

namespace rvm64::mock {
	_native void destroy_file(vm_buffer_t *data) {
		if (data) {
			if (data->address) {
				free((void*)data->address);
			}
			free((void*)data);
		}
	}

	_native vm_buffer_t *read_file() {
		BOOL success = false;
		DWORD bytes_read = 0;

		auto buffer = (vm_buffer_t*) malloc(sizeof(vm_buffer_t));

		HANDLE hfile = ctx->win32.NtCreateFile("./test.elf", GENERIC_READ, FILE_SHARE_READ, nullptr,
		                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hfile == INVALID_HANDLE_VALUE) {
			CSR_SET(nullptr, undefined, (uintptr_t)INVALID_HANDLE_VALUE, 0, 1);
			goto defer;
		}

		NTSTATUS status = 0;
		status = ctx->win32.NtGetFileSize(hfile, (LPDWORD) &buffer->size);

		if (status == INVALID_FILE_SIZE || buffer->size == 0) {
			CSR_SET(nullptr, undefined, INVALID_FILE_SIZE, 0, 1);
			vmcs->halt = 1;
			goto defer;
		}

		if (!ctx->win32.NtReadFile(hfile, (LPVOID) buffer->address, buffer->size, &bytes_read, NULL) ||
		    bytes_read != buffer->size) {
			CSR_SET(nullptr, undefined, 0, 0, 1);
			goto defer;
		}
		success = true;

	defer:
		if (hfile) {
			CloseHandle(hfile);
		}
		if (!success) {
			free(buffer);
			buffer = nullptr;
		}

		return buffer;
	}

	_native bool read_program_from_packet() {
		vm_buffer_t *data = read_file();
		if (!data) {
			return false;
		}

		data->size += VM_PROCESS_PADDING;

		rvm64::memory::memory_init(data->size);
		rvm64::elf::load_elf_image(data->address, data->size);
		rvm64::elf::patch_elf_imports();

		destroy_file(data);
		return true;
	}
};
#endif // MOCK_H
