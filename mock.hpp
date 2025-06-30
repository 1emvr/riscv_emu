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
			free(data);
		}
	}

	_native vm_buffer_t* read_file() {
		DWORD status = 0;
		DWORD bytes_read = 0;

		auto buffer = (vm_buffer_t*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(vm_buffer_t));
		HANDLE hfile = CreateFileA("./test.elf", GENERIC_READ, FILE_SHARE_READ, nullptr,
		                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hfile == INVALID_HANDLE_VALUE) {
			buffer->stat = GetLastError();
			return buffer;
		}

		status = GetFileSize(hfile, (LPDWORD)&buffer->size);
		if (status == INVALID_FILE_SIZE || buffer->size == 0) {
			buffer->stat = GetLastError();
			return buffer;
		}

		buffer->address = (uint8_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buffer->size);

		if (!ReadFile(hfile, buffer->address, buffer->size, &bytes_read, nullptr) ||
		    bytes_read != buffer->size) {
			buffer->stat = GetLastError();
			return buffer;
		}

		if (hfile) {
			CloseHandle(hfile);
		}
		return buffer;
	}

	_native void read_program_from_packet() {
		vm_buffer_t *data = read_file();
		if (!data) {
			CSR_SET_TRAP(nullptr, image_bad_load, ERROR_BUFFER_OVERFLOW, 0, 1);
		}
		if (data->stat) {
			CSR_SET_TRAP(nullptr, image_bad_load, data->stat, 0, 1);
		}

		data->size += VM_PROCESS_PADDING;

		rvm64::memory::memory_init(data->size);
		rvm64::elf::load_elf_image(data->address, data->size);

		// NOTE: if we want to cache the file for later use, then do not destroy_file
		// if (packet->cache == false) { ... }
		destroy_file(data);
	}
};
#endif // MOCK_H
