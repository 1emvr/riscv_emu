#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "mono.hpp"
#include "vmelf.h"
#include "vmmain.h"

#define PT_LOAD 1
#define ET_EXEC 2
#define ET_DYN  3

bool load_elf64_image(void) {
	if (!vmcs->program.address || !vmcs->process.address) {
		printf("Invalid VMCS or memory pointers\n");
		return false;
	}

    Elf64_Ehdr* e_head = (Elf64_Ehdr*)vmcs->program.address;
    if (!(e_head->e_ident[0] == 0x7F &&
    	  e_head->e_ident[1] == 'E' &&
          e_head->e_ident[2] == 'L' &&
          e_head->e_ident[3] == 'F'))
    {
        printf("Invalid ELF magic\n");
        return false;
    }

    if (e_head->e_type != ET_EXEC) {
        printf("Only ET_EXEC supported (no PIE)\n");
        return false;
    }

    Elf64_Phdr* p_heads = (Elf64_Phdr*)(vmcs->program.address + e_head->e_phoff);
    uint64_t base_vaddr = UINT64_MAX;

    for (int i = 0; i < e_head->e_phnum; i++) {
        if (p_heads[i].p_type == PT_LOAD && p_heads[i].p_filesz > 0) {
            if (p_heads[i].p_vaddr < base_vaddr) {
                base_vaddr = p_heads[i].p_vaddr;
            }
        }
    }

    if (base_vaddr == UINT64_MAX) {
        printf("No loadable segments found\n");
        return false;
    }

    for (int i = 0; i < e_head->e_phnum; i++) {
        Elf64_Phdr* ph = &p_heads[i];

    	if (ph->p_vaddr < base_vaddr) {
    		printf("Malformed ELF segment: integer underflow possible from base.\n");
    		return false;
    	}

        uint64_t offset = ph->p_vaddr - base_vaddr;
        if (ph->p_type != PT_LOAD) {
	        continue;
        }
        if (offset + ph->p_memsz > vmcs->process.size) {
            printf("Segment out of bounds: offset 0x%llx memsz 0x%llx > vmcs->process.address size 0x%zx - Consider increasing vram.\n", offset, ph->p_memsz, vmcs->process.size);
            return false;
        }

        void* src = (void*)vmcs->program.address + ph->p_offset;
        void* dst = (void*)vmcs->process.address + offset;

        memcpy(dst, src, ph->p_filesz);

        if (ph->p_memsz > ph->p_filesz) {
            memset((uint8_t*)dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        }
    }

    vmcs->pc = vmcs->process.address + (e_head->e_entry - base_vaddr);
    return true;
}

bool read_program_from_packet() {
	HANDLE hfile = ctx->win32.NtCreateFile("./test.o", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		printf("Failed to open file\n");
		return false;
	}

	DWORD size = 0;
	DWORD status = ctx->win32.NtGetFileSize(hfile, &size);
	if (status == INVALID_FILE_SIZE || size == 0) {
		printf("Invalid or empty file\n");
		return false;
	}

	void* elf_data = nullptr;
	if (!NT_SUCCESS(vmcs->reason = ctx->win32.NtAllocateVirtualMemory(NtCurrentProcess(), &elf_data, 0, (PSIZE_T)&size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		printf("Failed to allocate memory for ELF\n");
		return false;
	}

	DWORD bytesRead = 0;
	if (!ctx->win32.NtReadFile(hfile, elf_data, size, &bytesRead, NULL) || bytesRead != size) {
		printf("Failed to read file\n");
		free(elf_data);
		return false;
	}

	vmcs->program.address = (uintptr_t)elf_data;
	vmcs->program.size = size;

	// Load (relocate) ELF from elf_data into vmcs->process.address
	bool success = load_elf64_image();

	// Free the temporary ELF file buffer
	ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), &elf_data, (PSIZE_T)&size, MEM_RELEASE);
	vmcs->program.address = 0;
	vmcs->program.size = 0;

	return success;
}
#endif // MOCK_H