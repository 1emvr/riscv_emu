#ifndef MOCK_H
#define MOCK_H
#include <windows.h>
#include "mono.hpp"
#include "vmelf.h"
#include "vmcs.h"

#include <cstdint>
#include <cstring>
#include <cstdio>

#define PT_LOAD 1
#define ET_EXEC 2
#define ET_DYN  3

bool load_elf64_image(void* elf_base, vm_memory_t* vm_ram) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elf_base;

    if (!(ehdr->e_ident[0] == 0x7F && ehdr->e_ident[1] == 'E' &&
          ehdr->e_ident[2] == 'L' && ehdr->e_ident[3] == 'F')) {
        printf("Invalid ELF magic\n");
        return false;
    }

    if (ehdr->e_type != ET_EXEC) {
        printf("Only ET_EXEC supported (no PIE)\n");
        return false;
    }

    Elf64_Phdr* phdrs = (Elf64_Phdr*)((uint8_t*)elf_base + ehdr->e_phoff);
    uint64_t base_vaddr = UINT64_MAX;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD && phdrs[i].p_filesz > 0) {
            if (phdrs[i].p_vaddr < base_vaddr) {
                base_vaddr = phdrs[i].p_vaddr;
            }
        }
    }

    if (base_vaddr == UINT64_MAX) {
        printf("No loadable segments found\n");
        return false;
    }

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr* ph = &phdrs[i];
        uint64_t offset = ph->p_vaddr - base_vaddr;

        if (ph->p_type != PT_LOAD) {
	        continue;
        }
        if (offset + ph->p_memsz > vm_ram->size) {
            printf("Segment out of bounds: offset 0x%llx memsz 0x%llx > vm_ram size 0x%zx\n",
                   offset, ph->p_memsz, vm_ram->size);

            return false;
        }

        void* src = (uint8_t*)elf_base + ph->p_offset;
        void* dst = vm_ram->address + offset;
        memcpy(dst, src, ph->p_filesz);

        if (ph->p_memsz > ph->p_filesz) {
            memset((uint8_t*)dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        }
    }

    vmcs->pc = (uintptr_t)(vm_ram->address + (ehdr->e_entry - base_vaddr));
    return true;
}

bool read_program_from_packet() {

	HANDLE hfile = ctx->win32.NtCreateFile("./test.o", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
	if (!ctx->win32.NtReadFile(hfile, (void*)&vmcs->program.address, vmcs->program.size, &status, NULL)) {
		vmcs->reason = GetLastError();
		return false;
	}

	return load_elf64_image((void*)vmcs->program.address, (uint8_t*)vmcs->process.address);
}
#endif // MOCK_H