#ifndef VMELF_H
#define VMELF_H
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#include "vmlib.hpp"
#include "vmmain.hpp"
#include "rvni.hpp"

// Dynamic section tags
#define DT_NULL         0       // End of dynamic section
#define DT_PLTGOT		3		// the global offset table
#define DT_SYMTAB       6       // Pointer to symbol table
#define DT_STRTAB       5       // Pointer to string table
#define DT_SYMENT       11      // Size of one symbol table entry
#define DT_JMPREL       23      // Pointer to PLT relocation entries
#define DT_PLTRELSZ     2       // Size in bytes of PLT relocations
#define DT_PLTREL       20      // Type of PLT relocations (DT_RELA or DT_REL)
#define DT_RELA         7       // Address of relocation table with addends
#define DT_RELASZ       8       // Size of the DT_RELA table
#define DT_RELAENT      9       // Size of one DT_RELA entry
								//
#define EI_MAG0			0   // 0x7F
#define EI_MAG1			1   // 'E'
#define EI_MAG2			2   // 'L'
#define EI_MAG3			3   // 'F'
#define EI_CLAS			4   // File class
#define EI_DATA			5   // Data encoding
#define EI_VERS			6   // File version
#define EI_OSAB			7   // OS/ABI identification
#define EI_ABIVERSION	8 // ABI version
#define EI_PAD			9   // Start of padding bytes (up to 16)
#define EI_NIDENT		16  // Size of e_ident[]

#define ELFCLASS32		1
#define ELFCLASS64		2

#define ET_NONE			0 // No file type
#define ET_REL 			1 // Relocatable file
#define ET_EXEC			2 // Executable file
#define ET_DYN 			3 // Shared object file
#define ET_CORE			4 // Core file

#define EM_NONE			0
#define EM_RISC			243 // RISC-V target

#define PT_NULL			0
#define PT_LOAD			1
#define PT_DYNA			2
#define PT_INTE			3
#define PT_NOTE			4
#define PT_SHLI			5
#define PT_PHDR			6
#define PT_TLS 			7

#define SHT_NULL        0           // Inactive section header
#define SHT_PROGBITS    1           // Program-defined contents
#define SHT_SYMTAB      2           // Symbol table
#define SHT_STRTAB      3           // String table
#define SHT_RELA        4           // Relocation entries with addends
#define SHT_NOBITS      8           // Uninitialized data (.bss)
#define SHT_DYNSYM      11          // Dynamic linker symbol table
									//
#define SHN_UNDEF 		0	
#define SHN_LORESERVE   0xff00
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
#define SHN_ABS         0xfff1   // Absolute values, not affected by relocation
#define SHN_COMMON      0xfff2   // Common symbols
#define SHN_HIRESERVE   0xffff

#define R_RISCV_NONE          	0
#define R_RISCV_32             	1   // Direct 32-bit
#define R_RISCV_64             	2   // Direct 64-bit
#define R_RISCV_RELATIVE       	3   // Adjust by program base
#define R_RISCV_COPY           	4   // Copy symbol at runtime
#define R_RISCV_JUMP_SLOT      	5   // Create PLT entry
#define R_RISCV_TLS_DTPMOD32   	6
#define R_RISCV_TLS_DTPMOD64   	7
#define R_RISCV_TLS_DTPREL32   	8
#define R_RISCV_TLS_DTPREL64   	9
#define R_RISCV_TLS_TPREL32   	10
#define R_RISCV_TLS_TPREL64   	11
#define R_RISCV_BRANCH        	16   // PC-relative branch
#define R_RISCV_JAL           	17   // PC-relative jump (J-type)
#define R_RISCV_CALL          	18   // Call (with register save)
#define R_RISCV_CALL_PLT      	19   // PLT call
#define R_RISCV_GOT_HI20      	20   // High 20 bits of GOT address
#define R_RISCV_TLS_GOT_HI20  	21
#define R_RISCV_TLS_GD_HI20   	22
#define R_RISCV_PCREL_HI20    	23   // High 20 bits PC-relative
#define R_RISCV_PCREL_LO12_I  	24   // Low 12 bits for I-type
#define R_RISCV_PCREL_LO12_S  	25   // Low 12 bits for S-type
#define R_RISCV_HI20          	26   // High 20 bits of absolute address
#define R_RISCV_LO12_I        	27   // Low 12 bits of absolute address (I-type)
#define R_RISCV_LO12_S        	28   // Low 12 bits of absolute address (S-type)
#define R_RISCV_TPREL_HI20    	29
#define R_RISCV_TPREL_LO12_I  	30
#define R_RISCV_TPREL_LO12_S  	31
#define R_RISCV_RELAX         	51   // Instruction can be relaxed
#define R_RISCV_ALIGN         	52   // Alignment hint

#define ELF64_R_SYM(info)    	((info) >> 32)   // Extract symbol index from relocation info
#define ELF64_REL_TYPE(info)   	((info) & 0xFFFFFFFF)   // Extract relocation type from relocation info
#define ELF64_R_INFO(S, T) 		((((uint64_t)(S)) << 32) + (T))  // Construct relocation info from symbol index and type

#define ELF64_ST_BIND(info)   	((info) >> 4)
#define ELF64_ST_TYPE(info)   	((info) & 0xF)
#define ELF64_ST_INFO(B, T) 	(((B) << 4) + ((T) & 0xF))

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

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
} elf64_ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;   // Offset in file
    uint64_t p_vaddr;    // Virtual address in memory
    uint64_t p_paddr;
    uint64_t p_filesz;   // Bytes in file
    uint64_t p_memsz;    // Bytes in memory
    uint64_t p_align;
} elf64_phdr;

typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
} elf64_rela;

typedef struct {
    uint32_t st_name;
    uint8_t  st_info;
    uint8_t  st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} elf64_sym;

typedef struct {
    uint32_t sh_name;       // Offset to section name in the section header string table
    uint32_t sh_type;       // Section type (e.g., SHT_PROGBITS, SHT_SYMTAB)
    uint64_t sh_flags;      // Section attributes (e.g., SHF_ALLOC, SHF_EXECINSTR)
    uint64_t sh_addr;       // Virtual address in memory (for loaded sections)
    uint64_t sh_offset;     // Offset in the file
    uint64_t sh_size;       // Size of the section
    uint32_t sh_link;       // Link to another section (e.g., symbol table link)
    uint32_t sh_info;       // Additional section information (depends on type)
    uint64_t sh_addralign;  // Section alignment
    uint64_t sh_entsize;    // Entry size if section holds table (e.g., symbol table)
} elf64_shdr;


typedef struct {
    int64_t d_tag;
    union {
        uint64_t d_val;
        uint64_t d_ptr;
    } d_un;
} elf64_dyn;

// NOTE: I do not understand the elf format.
namespace rvm64::elf {
	_native void patch_elf_imports() {
		auto process = vmcs->process.address;
		auto ehdr = (elf64_ehdr*)process;

		if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
			CSR_SET_TRAP(nullptr, image_bad_type, 0, 0, 1);
		}

		auto phdrs = (elf64_phdr*) ((uint8_t*)(process) + ehdr->e_phoff);
		uint64_t dyn_vaddr = 0;
		uint64_t dyn_size = 0;

		for (int i = 0; i < ehdr->e_phnum; ++i) { 
			if (phdrs[i].p_type == PT_DYNA) {
				dyn_vaddr = phdrs[i].p_vaddr;
				dyn_size = phdrs[i].p_memsz;
				break;
			}
		}
		if (!dyn_vaddr || !dyn_size) {
			CSR_SET_TRAP(nullptr, image_bad_load, 0, 0, 1);
		}

		auto* dyn_entries = (elf64_dyn*) ((uint8_t*)process + dyn_vaddr); 
		uint64_t symtab_vaddr = 0, strtab_vaddr = 0, rela_plt_vaddr = 0, rela_plt_size = 0;

		// TODO: add support for .rela.dyn entries as well (R_RISCV_RELATIVE and others)
		for (elf64_dyn *dyn = dyn_entries; dyn->d_tag != DT_NULL; ++dyn) {
			switch (dyn->d_tag) {
				case DT_SYMTAB:   symtab_vaddr = dyn->d_un.d_ptr; break;
				case DT_STRTAB:   strtab_vaddr = dyn->d_un.d_ptr; break;
				case DT_JMPREL:   rela_plt_vaddr = dyn->d_un.d_ptr; break;
				case DT_PLTRELSZ: rela_plt_size = dyn->d_un.d_val; break;
				case DT_PLTREL: {
					if (dyn->d_un.d_val != DT_RELA) {
						CSR_SET_TRAP(nullptr, image_bad_load, 0, 0, 1);
					}
					break;
				}
				default: {
					break;
				}
			}
		}

		if (!symtab_vaddr || !strtab_vaddr || !rela_plt_vaddr || !rela_plt_size) {
			CSR_SET_TRAP(nullptr, image_bad_load, 0, 0, 1);
		}

		auto rela_entries 	= (elf64_rela*) ((uint8_t*)process + rela_plt_vaddr);
		auto symtab 		= (elf64_sym*) ((uint8_t*)process + symtab_vaddr);
		auto strtab 		= (const char*) ((uint8_t*)process + strtab_vaddr);
		size_t rela_count 	= rela_plt_size / sizeof(elf64_rela);

		for (size_t i = 0; i < rela_count; ++i) {
			uint32_t sym_idx = ELF64_R_SYM(rela_entries[i].r_info);
			uint32_t rel_type = ELF64_REL_TYPE(rela_entries[i].r_info);

			auto reloc_addr = (uint64_t*) ((uint8_t*)process + rela_entries[i].r_offset);
			const char *sym_name = strtab + symtab[sym_idx].st_name;

			if (rel_type != R_RISCV_JUMP_SLOT && rel_type != R_RISCV_CALL_PLT) {
				continue;
			}
			void *win_func = rvm64::rvni::windows_thunk_resolver(sym_name);
			if (!win_func) {
				CSR_SET_TRAP(nullptr, image_bad_symbol, 0, (uintptr_t)sym_name, 1);
			}

			*reloc_addr = (uint64_t)(win_func);
		}

		auto shdrs = (elf64_shdr*) ((uint8_t*)process + ehdr->e_shoff);
		if (ehdr->e_shstrndx != SHN_UNDEF) {
			auto& strtab_hdr = shdrs[ehdr->e_shstrndx];
			strtab = (const char*)((uint8_t*)process + strtab_hdr.sh_offset);
		}

		for (int i = 0; i < ehdr->e_shnum; ++i) {
			const auto& shdr = shdrs[i]; 

			if (shdr.sh_type == SHT_PROGBITS && strcmp(strtab + shdr.sh_name, ".plt") == 0) {
				vmcs->process.plt.start = shdr.sh_addr;
				vmcs->process.plt.end   = shdr.sh_addr + shdr.sh_size;
				break;
			}
		}
	}

	_native void load_elf_image(void* image_data, size_t image_size) {
		auto ehdr = (elf64_ehdr*)(image_data);

		if (ehdr->e_ident[0] != 0x7F ||
		    ehdr->e_ident[1] != 'E' ||
		    ehdr->e_ident[2] != 'L' ||
		    ehdr->e_ident[3] != 'F') {
			CSR_SET_TRAP(nullptr, image_bad_type, 0, 0, 1);
		}
		if (ehdr->e_ident[EI_CLAS] != ELFCLASS64 || ehdr->e_machine != EM_RISC) {
			CSR_SET_TRAP(nullptr, image_bad_type, 0, 0, 1);
		}

		elf64_phdr* phdrs = (elf64_phdr*) ((uint8_t*)image_data + ehdr->e_phoff);
		uint64_t base = UINT64_MAX;
		uint64_t limit = 0;

		for (int i = 0; i < ehdr->e_phnum; ++i) {
			if (phdrs[i].p_type == PT_LOAD) {
				base = MIN(base, phdrs[i].p_vaddr);
				limit = MAX(limit, phdrs[i].p_vaddr + phdrs[i].p_memsz);
			}
		}
		if (base == UINT64_MAX || limit <= base) {
			CSR_SET_TRAP(nullptr, image_bad_load, 0, 0, 1);
		}

		for (int i = 0; i < ehdr->e_phnum; ++i) {
			if (phdrs[i].p_type != PT_LOAD) {
				continue;
			}
			void *dest = (uint8_t*)vmcs->process.address + (phdrs[i].p_vaddr - base);
			void *src = (uint8_t*)image_data + phdrs[i].p_offset;

			x_memcpy(dest, src, phdrs[i].p_filesz);

			if (phdrs[i].p_memsz > phdrs[i].p_filesz) {
				x_memset((uint8_t*)dest + phdrs[i].p_filesz, 0, phdrs[i].p_memsz - phdrs[i].p_filesz);
			}
		}

		vmcs->process.size = image_size;
		vmcs->process.base_vaddr = base;
		vmcs->process.entry = vmcs->process.address + (ehdr->e_entry - base);

		vmcs->pc = vmcs->process.entry;
		patch_elf_imports();
	}
};
#endif // VMELF_H
