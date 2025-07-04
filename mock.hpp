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
		if (!data) {
			return;
		}
		if (data->address) {
			HeapFree(GetProcessHeap(), 0, data->address);
			data->address = nullptr;
			data->size = 0;
		}

		HeapFree(GetProcessHeap(), 0, data);
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

		data->size = GetFileSize(handle, nullptr);
		if (data->size == INVALID_FILE_SIZE || data->size == 0) {
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
};
#endif // MOCK_H
