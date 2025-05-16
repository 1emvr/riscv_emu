#ifndef VMELF_H
#define VMELF_H
#define EI_NIDENT 16
#define PT_LOAD 1

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
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;   // Offset in file
    uint64_t p_vaddr;    // Virtual address in memory
    uint64_t p_paddr;
    uint64_t p_filesz;   // Bytes in file
    uint64_t p_memsz;    // Bytes in memory
    uint64_t p_align;
} Elf64_Phdr;


#endif //VMELF_H
