#include <windows.h>
#include <stdint.h>

#include "vmcs.h"
#include "vmctx.h"
#include "vmmem.h"

__data hexane *ctx = { };
__data vmcs_t vmcs = { };
__data uintptr_t __stack_cookie = { };

struct opcode {
    uint8_t op;
    typenum type;
};
__rdata const opcode encoding[] = {
    { 0b1010011, rtype  }, { 0b1000011, rtype  }, { 0b0110011, rtype  },
    { 0b1000111, r4type }, { 0b1001011, r4type }, { 0b1001111, r4type },
    { 0b0000011, itype  }, { 0b0001111, itype  }, { 0b1100111, itype  },
    { 0b0010011, itype  }, { 0b1110011, itype  }, { 0b0100011, stype  },
    { 0b0100111, stype  }, { 0b1100011, btype  }, { 0b0010111, utype  },
    { 0b0110111, utype  }, { 0b1101111, jtype  },
};

namespace rvm64 {
    __function int64_t vm_main(void) {
        while(!vmcs.halt) {
            if (read_program_from_packet(vmcs.program)) {
                break;
            }
        };

        rvm64::memory::vm_init();
        rvm64::context::vm_entry();
        rvm64::memory::vm_end();
        return vmcs.reason;
    }
};



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
        vmcs.halt = 1;
        vmcs.reason = illegal_op;
        return;
    }

    reg_write(int32_t, rd, (v1 << shamt));
}

__function void rv_srliw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;
    int32_t shamt = imm_11_0 & 0x1F;

    reg_read(int32_t, rs1, v1);

    if ((imm_11_0 >> 5) != 0) {
        vmcs.halt = 1;
        vmcs.reason = illegal_op;
        return;
    }

    reg_write(int32_t, rd, (v1 >> shamt));
}

__function void rv_sraiw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    int32_t v1 = 0;
    uint8_t shamt = imm_11_0 & 0x1F;

    reg_read(int32_t, rs1, v1);

    if ((imm_11_0 >> 5) != 0) {
        vmcs.halt = 1;
        vmcs.reason = illegal_op;
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
    uintptr_t retval = 0;

    reg_read(uintptr_t, rs1, address);
    address += (intptr_t)(int16_t)imm_11_0;
    address &= ~1;

    ip_read(retval);
    retval += 4;

    reg_write(uintptr_t, rd, retval);
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
    rvm64::context::save_vm_context();
    // hyperv_switch();
    rvm64::context::restore_vm_context();
}

__function void rv_ebreak(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
    /*
                   Description
                   Used by debuggers to cause control to be transferred back to a debugging environment.
                   It generates a breakpoint exception and performs no other operation.

                   Implementation
                   RaiseException(Breakpoint)
                   */
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
                case 0b000: return rvm64::operation::__handler[_addi];
                case 0b010: return rvm64::operation::__handler[_slti];
                case 0b011: return rvm64::operation::__handler[_sltiu];
                case 0b100: return rvm64::operation::__handler[_xori];
                case 0b110: return rvm64::operation::__handler[_ori];
                case 0b111: return rvm64::operation::__handler[_andi];
                case 0b001: return rvm64::operation::__handler[_slli];
                case 0b101: switch(imm_mask) {
                    case 0b0000000: return rvm64::operation::__handler[_srli];
                    case 0b0100000: return rvm64::operation::__handler[_srai];
                }
            }
            case 0b0011011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[_addiw];
                case 0b001: return rvm64::operation::__handler[_slliw];
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_srliw];
                    case 0b0100000: return rvm64::operation::__handler[_sraiw];
                }
            }
            case 0b0000011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[_lb];
                case 0b001: return rvm64::operation::__handler[_lh];
                case 0b010: return rvm64::operation::__handler[_lw];
                case 0b100: return rvm64::operation::__handler[_lbu];
                case 0b101: return rvm64::operation::__handler[_lhu];
                case 0b110: return rvm64::operation::__handler[_lwu];
                case 0b011: return rvm64::operation::__handler[_ld];
            }
        } 

        // R_TYPE
        case rtype: switch(opcode) {
            case 0b1010011: switch(func7) {
                case 0b0000001: return rvm64::operation::__handler[_fadd_d];
                case 0b0000101: return rvm64::operation::__handler[_fsub_d];
                case 0b0001001: return rvm64::operation::__handler[_fmul_d];
                case 0b0001101: return rvm64::operation::__handler[_fdiv_d];
                case 0b0101101: return rvm64::operation::__handler[_fsqrt_d];
                case 0b1111001: return rvm64::operation::__handler[_fmv_d_x];
                case 0b0100000: switch(rs2) {
                    case 0b00001: return rvm64::operation::__handler[_fcvt_s_d];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_s_q];
                }
                case 0b0100001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[_fcvt_d_s];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_d_q];
                }
                case 0b1100001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[_fcvt_w_d];
                    case 0b00001: return rvm64::operation::__handler[_fcvt_wu_d];
                    case 0b00010: return rvm64::operation::__handler[_fcvt_l_d];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_lu_d];
                }
                case 0b1101001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[_fcvt_d_w];
                    case 0b00001: return rvm64::operation::__handler[_fcvt_d_wu];
                    case 0b00010: return rvm64::operation::__handler[_fcvt_d_l];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_d_lu];
                }
                case 0b0010001: switch(func3) {
                    case 0b000: return rvm64::operation::__handler[_fsgnj_d];
                    case 0b001: return rvm64::operation::__handler[_fsgnjn_d];
                    case 0b010: return rvm64::operation::__handler[_fsgnjx_d];
                }
                case 0b0010101: switch(func3) {
                    case 0b000: return rvm64::operation::__handler[_fmin_d];
                    case 0b001: return rvm64::operation::__handler[_fmax_d];
                }
                case 0b1010001: switch(func3) {
                    case 0b010: return rvm64::operation::__handler[_feq_d];
                    case 0b001: return rvm64::operation::__handler[_flt_d];
                    case 0b000: return rvm64::operation::__handler[_fle_d];
                }
                case 0b1110001: switch(func3) {
                    case 0b001: return rvm64::operation::__handler[_fclass_d];
                    case 0b000: return rvm64::operation::__handler[_fmv_x_d];
                }
            }
            case 0b0101111: switch(func3) {
                case 0b010: switch(func5) {
                    case 0b00010: return rvm64::operation::__handler[_lrw];
                    case 0b00011: return rvm64::operation::__handler[_scw];
                    case 0b00001: return rvm64::operation::__handler[_amoswap_w];
                    case 0b00000: return rvm64::operation::__handler[_amoadd_w];
                    case 0b00100: return rvm64::operation::__handler[_amoxor_w];
                    case 0b01100: return rvm64::operation::__handler[_amoand_w];
                    case 0b01000: return rvm64::operation::__handler[_amoor_w];
                    case 0b10000: return rvm64::operation::__handler[_amomin_w];
                    case 0b10100: return rvm64::operation::__handler[_amomax_w];
                    case 0b11000: return rvm64::operation::__handler[_amominu_w];
                    case 0b11100: return rvm64::operation::__handler[_amomaxu_w];
                }
                case 0b011: switch(func5) {
                    case 0b00010: return rvm64::operation::__handler[_lrd];
                    case 0b00011: return rvm64::operation::__handler[_scd];
                    case 0b00001: return rvm64::operation::__handler[_amoswap_d];
                    case 0b00000: return rvm64::operation::__handler[_amoadd_d];
                    case 0b00100: return rvm64::operation::__handler[_amoxor_d];
                    case 0b01100: return rvm64::operation::__handler[_amoand_d];
                    case 0b01000: return rvm64::operation::__handler[_amoor_d];
                    case 0b10000: return rvm64::operation::__handler[_amomin_d];
                    case 0b10100: return rvm64::operation::__handler[_amomax_d];
                    case 0b11000: return rvm64::operation::__handler[_amominu_d];
                    case 0b11100: return rvm64::operation::__handler[_amomaxu_d];
                }
            }
            case 0b0111011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_addw];
                    case 0b0100000: return rvm64::operation::__handler[_subw];
                    case 0b0000001: return rvm64::operation::__handler[_mulw];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_srlw];
                    case 0b0100000: return rvm64::operation::__handler[_sraw];
                    case 0b0000001: return rvm64::operation::__handler[_divuw];
                }
                case 0b001: return rvm64::operation::__handler[_sllw];
                case 0b100: return rvm64::operation::__handler[_divw];
                case 0b110: return rvm64::operation::__handler[_remw];
                case 0b111: return rvm64::operation::__handler[_remuw];
            }
            case 0b0110011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_add];
                    case 0b0100000: return rvm64::operation::__handler[_sub];
                    case 0b0000001: return rvm64::operation::__handler[_mul];
                }
                case 0b001: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_sll];
                    case 0b0000001: return rvm64::operation::__handler[_mulh];
                }
                case 0b010: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_slt];
                    case 0b0000001: return rvm64::operation::__handler[_mulhsu];
                }
                case 0b011: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_sltu];
                    case 0b0000001: return rvm64::operation::__handler[_mulhu];
                }
                case 0b100: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_xor];
                    case 0b0000001: return rvm64::operation::__handler[_div];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_srl];
                    case 0b0100000: return rvm64::operation::__handler[_sra];
                    case 0b0000001: return rvm64::operation::__handler[_divu];
                }
                case 0b110: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_or];
                    case 0b0000001: return rvm64::operation::__handler[_rem];
                }
                case 0b111: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_and];
                    case 0b0000001: return rvm64::operation::__handler[_remu];
                }
            }
        } 
        // S TYPE
        case stype: switch(opcode) {
            case 0b0100011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[_sb];
                case 0b001: return rvm64::operation::__handler[_sh];
                case 0b010: return rvm64::operation::__handler[_sw];
                case 0b011: return rvm64::operation::__handler[_sd];
            }
            case 0b0100111: switch(func3) {
                case 0b010: return rvm64::operation::__handler[_fsw];
                case 0b011: return rvm64::operation::__handler[_fsd];
                case 0b100: return rvm64::operation::__handler[_fsq];
            }
        }
        // B TYPE
        case btype: switch(func3) {
            case 0b000: return rvm64::operation::__handler[_beq];
            case 0b001: return rvm64::operation::__handler[_bne];
            case 0b100: return rvm64::operation::__handler[_blt];
            case 0b101: return rvm64::operation::__handler[_bge];
            case 0b110: return rvm64::operation::__handler[_bltu];
            case 0b111: return rvm64::operation::__handler[_bgeu];
        }
        // U TYPE
        case utype: switch(opcode) {
            case 0b0110111: return rvm64::operation::__handler[_lui];
            case 0b0010111: return rvm64::operation::__handler[_auipc];
        }
        // J TYPE
        case jtype: switch(opcode) {
            case 0b1101111: return rvm64::operation::__handler[_jal];
        }
        // R4 TYPE
        case r4type: {}
    }
}

