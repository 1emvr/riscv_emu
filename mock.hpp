#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <string.h>

#include "vmmain.hpp"
#include "vmcommon.hpp"
#include "vmmem.hpp"
#include "vmelf.hpp"

namespace rvm64::mock {
	_native void destroy_file(vm_buffer_t *data) {
		if (data) {
			if (data->address) {
				HeapFree(GetProcessHeap(), 0, data->address);
			}
			HeapFree(GetProcessHeap(), 0, data);
		}
	}

	_native vm_buffer_t* read_file() {
		DWORD status = 0;
		DWORD bytes_read = 0;

		auto data = (vm_buffer_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(vm_buffer_t));
		if (!data) {
			return nullptr;
		}
		HANDLE handle = CreateFileA("./test.elf", GENERIC_READ, FILE_SHARE_READ, nullptr,
									OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (handle == INVALID_HANDLE_VALUE) {
			data->stat = GetLastError();
			return data;
		}

		status = GetFileSize(handle, (LPDWORD)&data->size);
		if (status == INVALID_FILE_SIZE || data->size == 0) {
			data->stat = GetLastError();
			return data;
		}

		data->address = (uint8_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, data->size);

		if (!ReadFile(handle, data->address, data->size, &bytes_read, nullptr) ||
		    bytes_read != data->size) {
			data->stat = GetLastError();
			return data;
		}
		if (handle) {
			CloseHandle(handle);
		}

		return data;
	}

	//NOTE: consider returning the packet data from here to keep memory/elf init separated.
	_native void read_program_from_packet() {
		vm_buffer_t *data = read_file();
		if (data == nullptr) {
			CSR_SET_TRAP(nullptr, image_bad_load, STATUS_NO_MEMORY, 0, 1);
		}
		if (data->stat) {
			CSR_SET_TRAP(nullptr, image_bad_load, data->stat, 0, 1);
		}

		data->size += VM_PROCESS_PADDING;

		rvm64::memory::memory_init(data->size);
		rvm64::elf::load_elf_image(data->address, data->size);

		// NOTE: if we want to cache the file for later use, then do not destroy_file
		// if (packet->cache == false) { destroy_file(data); }
		destroy_file(data);
	}
};
#endif // MOCK_H
