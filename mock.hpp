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

	_native vm_buffer_t* read_file() {
		NTSTATUS status = 0;
		DWORD bytes_read = 0;

		auto buffer = (vm_buffer_t*) malloc(sizeof(vm_buffer_t));
		HANDLE hfile = ctx->win32.NtCreateFile("./test.elf", GENERIC_READ, FILE_SHARE_READ, nullptr,
		                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hfile == INVALID_HANDLE_VALUE) {
			CSR_SET_TRAP(nullptr, undefined, (uintptr_t)INVALID_HANDLE_VALUE, 0, 1);
		}

		status = ctx->win32.NtGetFileSize(hfile, (LPDWORD)&buffer->size);
		if (status == INVALID_FILE_SIZE || buffer->size == 0) {
			CSR_SET_TRAP(nullptr, image_bad_load, INVALID_FILE_SIZE, 0, 1);
		}

		buffer->address = (uint8_t*)malloc(buffer->size);

		if (!ctx->win32.NtReadFile(hfile, (LPVOID) buffer->address, buffer->size, &bytes_read, NULL) ||
		    bytes_read != buffer->size) {
			CSR_SET_TRAP(nullptr, image_bad_load, 0, 0, 1);
		}

		if (hfile) {
			CloseHandle(hfile);
		}
		return buffer;
	}

	_native void read_program_from_packet() {
		vm_buffer_t *data = read_file();
		if (!data) {
			CSR_SET_TRAP(nullptr, image_bad_load, 0, 0, 1);
		}

		data->size += VM_PROCESS_PADDING;

		rvm64::memory::memory_init(data->size);
		rvm64::elf::load_elf_image(data->address, data->size);

		// NOTE: if we want to cache the file for later use, then do not destroy_file
		destroy_file(data);
	}
};
#endif // MOCK_H
