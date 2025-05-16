#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include "vmelf.h"
#include "mono.hpp"
#include "vmcs.h"

bool load_elf64_image(void* elf_base, uint8_t* vm_ram_base) {
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elf_base;

	// Validate ELF magic
	if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
		return false;
		}

	Elf64_Phdr* phdrs = (Elf64_Phdr*)((uint8_t*)elf_base + ehdr->e_phoff);
	uint64_t base_vaddr = UINT64_MAX;

	// Compute base virtual address (used for relocation)
	for (int i = 0; i < ehdr->e_phnum; i++) {
		if (phdrs[i].p_type == PT_LOAD && phdrs[i].p_filesz > 0) {
			if (phdrs[i].p_vaddr < base_vaddr) {
				base_vaddr = phdrs[i].p_vaddr;
			}
		}
	}

	// Load all PT_LOAD segments into VM memory
	for (int i = 0; i < ehdr->e_phnum; i++) {
		Elf64_Phdr* ph = &phdrs[i];
		if (ph->p_type != PT_LOAD) continue;

		void* src = (uint8_t*)elf_base + ph->p_offset;
		void* dst = vm_ram_base + (ph->p_vaddr - base_vaddr);

		// Copy segment
		memcpy(dst, src, ph->p_filesz);

		// Zero out the rest if memsz > filesz
		if (ph->p_memsz > ph->p_filesz) {
			memset((uint8_t*)dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
		}
	}

	// Set VM entry point
	vmcs->pc = (uintptr_t)(vm_ram_base + (ehdr->e_entry - base_vaddr));
	return true;
}

bool read_program_from_packet() {

	HANDLE hfile = ctx->win32.NtCreateFile("./test.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD size = 0;
	DWORD status = ctx->win32.NtGetFileSize(hfile, &size);
	if (status == INVALID_FILE_SIZE) {
		return false;
	}

	vmcs->program.address = vmcs->process.address;
	vmcs->program.size = size;

	if (vmcs->process.size < vmcs->program.size) {
		return false;
	}
	return ctx->win32.NtReadFile(hfile, (void*)&vmcs->program.address, vmcs->program.size, &status, NULL);
	// TODO: file mapping here?
}
#endif // MOCK_H