#ifndef VMELF_H
#define VMELF_H

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((uint32_t)(i))

#define EI_NIDENT 16
#define PT_LOAD 1
#define ET_EXEC 2
#define ET_DYN  3

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <unordered_map>

#include "vmmain.hpp"
#include "rvni.hpp"


typedef struct {
    uint8_t  e_ident[EI_NIDENT]; // ELF magic, class, data, etc.
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;    // Virtual address of entry point
    uint64_t e_phoff;    // Program header offset
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;    // Number of program headers
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} e_ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;   // Offset in file
    uint64_t p_vaddr;    // Virtual address in memory
    uint64_t p_paddr;
    uint64_t p_filesz;   // Bytes in file
    uint64_t p_memsz;    // Bytes in memory
    uint64_t p_align;
} e_phdr;

typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
} e_rel;

typedef struct {
    uint32_t st_name;
    uint8_t  st_info;
    uint8_t  st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} e_sym;

typedef struct {
    int64_t d_tag;
    union {
        uint64_t d_val;
        uint64_t d_ptr;
    } d_un;
} e_dyn;

namespace rvm64::elf {
    _function bool load_elf64_image(void) {
        if (!vmcs->program.address || !vmcs->process.address) {
            return false;
        }

        e_ehdr* e_head = (e_ehdr*)vmcs->program.address;
        if (!(e_head->e_ident[0] == 0x7F &&
              e_head->e_ident[1] == 'E' &&
              e_head->e_ident[2] == 'L' &&
              e_head->e_ident[3] == 'F'))
        {
            return false;
        }

        if (e_head->e_type != ET_EXEC) {
            return false;
        }

        e_phdr* p_heads = (e_phdr*)(vmcs->program.address + e_head->e_phoff);
        uint64_t base_vaddr = UINT64_MAX; // TODO: consider changing this to UINTPTR_MAX

        for (int i = 0; i < e_head->e_phnum; i++) {
            if (p_heads[i].p_type == PT_LOAD && p_heads[i].p_filesz > 0) {
                if (p_heads[i].p_vaddr < base_vaddr) {
                    base_vaddr = p_heads[i].p_vaddr;
                }
            }
        }

        if (base_vaddr == UINT64_MAX) {
            return false;
        }

        for (int i = 0; i < e_head->e_phnum; i++) {
            e_phdr *ph = &p_heads[i];

            if (ph->p_vaddr < base_vaddr) { // NOTE: do we really need to check this twice?
                return false;
            }

            uint64_t offset = ph->p_vaddr - base_vaddr;
            if (ph->p_type != PT_LOAD) {
                continue;
            }

            if (offset + ph->p_memsz > vmcs->process.size) {
                printf("ERROR: Segment out of bounds: offset 0x%llx memsz 0x%llx > vmcs->process.address size 0x%zx - Consider increasing vram.\n", offset, ph->p_memsz, vmcs->process.size);

				vmcs->halt = 1;
				vmcs->reason = vm_stack_overflow;
                return false;
            }

            void *src = (void*) (vmcs->program.address + ph->p_offset);
            void *dst = (void*) (vmcs->process.address + offset);

            memcpy(dst, src, ph->p_filesz);

            if (ph->p_memsz > ph->p_filesz) {
                memset((uint8_t*) (dst + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
            }
        }

        vmcs->pc = vmcs->process.address + (e_head->e_entry - base_vaddr);
        return true;
    }

	_function bool patch_elf64_imports(void) {
		e_ehdr* ehdr = (e_ehdr*)vmcs->process.address;

		if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
			return false;
		}

		e_phdr* phdrs = (e_phdr*)((uint8_t*)vmcs->process.address + ehdr->e_phoff);

		uint64_t dyn_vaddr = 0;
		size_t dyn_size = 0;

		for (int i = 0; i < ehdr->e_phnum; i++) {
			if (phdrs[i].p_type == PT_DYNAMIC) {
				dyn_vaddr = phdrs[i].p_vaddr;
				dyn_size = phdrs[i].p_memsz;
				break;
			}
		}
		if (!dyn_vaddr || !dyn_size) {
			return false;
		}

		e_dyn* dyn_entries = (e_dyn*)((uint8_t*)vmcs->process.address + dyn_vaddr);

		uint64_t symtab_vaddr = 0, strtab_vaddr = 0, relplt_vaddr = 0;
		size_t relplt_size = 0, syment_size = 0;

		for (e_dyn* dyn = dyn_entries; dyn->d_tag != DT_NULL; dyn++) {
			switch (dyn->d_tag) {
				case DT_SYMTAB: 	symtab_vaddr = dyn->d_un.d_ptr; break;
				case DT_STRTAB: 	strtab_vaddr = dyn->d_un.d_ptr; break;
				case DT_SYMENT: 	syment_size = dyn->d_un.d_val;  break;
				case DT_JMPREL: 	relplt_vaddr = dyn->d_un.d_ptr; break;
				case DT_PLTRELSZ: 	relplt_size = dyn->d_un.d_val;  break;
				case DT_PLTREL:
									{
										if (dyn->d_un.d_val != DT_REL) {
											printf("ERROR: Only DT_REL supported in PLT relocations.\n");
											return false;
										}
										break;
									}
			}
		}

		if (!symtab_vaddr || !strtab_vaddr || !relplt_vaddr || !relplt_size) {
			return false;
		}

		e_rel* rel_entries = (e_rel*)((uint8_t*)vmcs->process.address + relplt_vaddr);
		e_sym* symtab = (e_sym*)((uint8_t*)vmcs->process.address + symtab_vaddr);

		const char* strtab = (const char*)((uint8_t*)vmcs->process.address + strtab_vaddr);
		size_t rel_count = relplt_size / sizeof(e_rel);

		for (size_t i = 0; i < rel_count; i++) {
			uint32_t sym_idx = ELF64_R_SYM(rel_entries[i].r_info);
			uint32_t r_type  = ELF64_R_TYPE(rel_entries[i].r_info);

			const char* sym_name = strtab + symtab[sym_idx].st_name;
			void* win_func = rvm64::rvni::windows_thunk_resolver(sym_name);

			if (!win_func) {
				printf("WARN: unresolved import: %s\n", sym_name);
				continue;
			}
			
			// NOTE: patching .got/.plt with win32 wrapper
			void** reloc_addr = (void**)((uint8_t*)vmcs->process.address + rel_entries[i].r_offset);
			*reloc_addr = win_func; // PATCH
		}

		return true;
	}
};
#endif // VMELF_H
