#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstring>

#include "vmmain.hpp"
#include "vmcommon.hpp"
#include "vmmem.hpp"
#include "vmelf.hpp"

namespace rvm64::mock {
	_native vm_buffer *read_file() {
		BOOL success = false;
		DWORD bytes_read = 0;

		auto buffer = (vm_buffer *) malloc(sizeof(vm_buffer));

		HANDLE hfile = ctx->win32.NtCreateFile("./test.elf", GENERIC_READ, FILE_SHARE_READ, nullptr,
		                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hfile == INVALID_HANDLE_VALUE) {
			csr_set(nullptr, undefined_error, 0, 0, 1);
			goto defer;
		}

		NTSTATUS status = 0;
		status = ctx->win32.NtGetFileSize(hfile, (LPDWORD) &buffer->size);

		if (status == INVALID_FILE_SIZE || buffer->size == 0) {
			csr_set(nullptr, undefined_error, INVALID_FILE_SIZE, 0, 1);
			vmcs->halt = 1;
			goto defer;
		}

		if (!ctx->win32.NtReadFile(hfile, (LPVOID) buffer->address, buffer->size, &bytes_read, NULL) ||
		    bytes_read != buffer->size) {
			csr_set(nullptr, undefined_error, 0, 0, 1);
			goto defer;
		}

		success = true;
	defer:
		if (hfile) {
			CloseHandle((HANDLE) hfile);
		}
		if (!success) {
			free(buffer);
			buffer = nullptr;
		}

		return buffer;
	}

	_native bool read_program_from_packet() {
		vm_buffer *data = read_file();
		if (!data) {
			return false;
		}

		data->size += VM_PROCESS_PADDING;

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
