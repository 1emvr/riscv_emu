#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include "mono.hpp"

bool read_program_from_packet(uintptr_t pointer) {
	HEXANE;
	
	HANDLE hfile = ctx->win32.NtCreateFile("./test.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD size = 0;
	DWORD status = ctx->win32.NtGetFileSize(hfile, &size);
	if (status == INVALID_FILE_SIZE) {
		return false;
	}

	return ctx->win32.NtReadFile(hfile, (void*)&pointer, size, &status, NULL);
	// TODO: file mapping likely needs to be done here.
}
#endif // MOCK_H