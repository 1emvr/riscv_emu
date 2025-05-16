#include "mono.hpp"
#include "vmmem.h"
#include "vmcs.h"

namespace rvm64::operation {
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

    // i-type
    namespace itype {
        __function void rv_lrw(uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t value = 0;

            reg_read(uintptr_t, rs1, address);
            mem_read(int32_t, address, value);

            reg_write(int64_t, rd, value);
            rvm64::memory::vm_set_load_rsv(address);
        }

        __function void rv_lrd(uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t value = 0;

            reg_read(uintptr_t, rs1, address);
            rvm64::memory::vm_set_load_rsv(address);

            mem_read(int64_t, address, value);
            reg_write(int64_t, rd, value);
        }

        __function void rv_fmv_d_x(uint8_t rs1, uint8_t rd) {
            int64_t v1 = 0;

            reg_read(int64_t, rs1, v1);
            reg_write(int64_t, rd, v1);
        }

        __function void rv_fcvt_s_d(uint8_t rs1, uint8_t rd) {
            float v1 = 0;

            reg_read(double, rs1, v1);
            reg_write(float, rd, v1);
        }

        __function void rv_fcvt_d_s(uint8_t rs1, uint8_t rd) {
            double v1 = 0;

            reg_read(float, rs1, v1);
            reg_write(double, rd, v1);
        }

        __function void rv_fcvt_w_d(uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0;

            reg_read(double, rs1, v1);
            reg_write(int32_t, rd, v1);
        }

        __function void rv_fcvt_wu_d(uint8_t rs1, uint8_t rd) {
            uint32_t v1 = 0;

            reg_read(double, rs1, v1);
            reg_write(uint32_t, rd, v1);
        }

        __function void rv_fcvt_d_w(uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0;

            reg_read(int32_t, rs1, v1);
            reg_write(double, rd, v1);
        }

        __function void rv_fcvt_d_wu(uint8_t rs1, uint8_t rd) {
            uint32_t v1 = 0;

            reg_read(uint32_t, rs1, v1);
            reg_write(double, rd, v1);
        }

        __function void rv_fclass_d(uint8_t rs1, uint8_t rd) {
            double v1 = 0;
            union {
                double d;
                int64_t u;
            } converter;

            reg_read(double, rs1, v1);
            converter.d = v1;

            uint64_t exponent = (converter.u >> 52) & 0x7FF;
            uint64_t fraction = converter.u & 0xFFFFFFFFFFFFF;
            uint64_t sign = converter.u >> 63;

            if (exponent == 0x7FF) {
                if (fraction == 0) {
                    if (sign == 0) {
                        reg_write(int, rd, 0x7); // +inf
                    } else {
                        reg_write(int, rd, 0x0); // -inf
                    }
                } else {
                    if (fraction & (1LL << 51)) {
                        reg_write(int, rd, 0x8); // quiet NaN
                    } else {
                        reg_write(int, rd, 0x9); // signaling NaN
                    }
                }
            } else if (exponent == 0) {
                if (fraction == 0) {
                    if (sign == 0) {
                        reg_write(int, rd, 0x4); // +0
                    } else {
                        reg_write(int, rd, 0x3); // -0
                    }
                } else {
                    if (sign == 0) {
                        reg_write(int, rd, 0x5); // +subnormal
                    } else {
                        reg_write(int, rd, 0x2); // -subnormal
                    }
                }
            } else {
                if (sign == 0) {
                    reg_write(int, rd, 0x6); // +normal
                } else {
                    reg_write(int, rd, 0x1); // -normal
                }
            }
        }

        __function void rv_addi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0;
            intptr_t imm = (intptr_t) (int16_t) imm_11_0;

            reg_read(intptr_t, rs1, v1);
            reg_write(intptr_t, rd, (v1 + imm));
        }

        __function void rv_slti(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0;
            intptr_t imm = (intptr_t) (int16_t) imm_11_0;

            reg_read(intptr_t, rs1, v1);
            reg_write(intptr_t, rd, ((v1 < imm) ? 1 : 0));
        }

        __function void rv_sltiu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0;
            uintptr_t imm = (uintptr_t) (uint16_t) imm_11_0;

            reg_read(uintptr_t, rs1, v1);
            reg_write(uintptr_t, rd, ((v1 < imm) ? 1 : 0));
        }

        __function void rv_xori(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0;
            intptr_t imm = (intptr_t) (int16_t) imm_11_0;

            reg_read(intptr_t, rs1, v1);
            reg_write(intptr_t, rd, (v1 ^ imm));
        }

        __function void rv_ori(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0;
            intptr_t imm = (intptr_t) (int16_t) imm_11_0;

            reg_read(intptr_t, rs1, v1);
            reg_write(intptr_t, rd, (v1 | imm));
        }

        __function void rv_andi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0;
            intptr_t imm = (intptr_t) (int16_t) imm_11_0;

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
            int32_t imm = (int32_t) (int16_t) imm_11_0;

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
            address += (intptr_t) (int16_t) imm_11_0;

            mem_read(int8_t, address, v1);
            reg_write(int8_t, rd, v1);
        }

        __function void rv_lh(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            int16_t v1 = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            address += (intptr_t) (int16_t) imm_11_0;

            mem_read(int16_t, address, v1);
            reg_write(intptr_t, rd, v1);
        }

        __function void rv_lw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            address += (intptr_t) (int16_t) imm_11_0;

            mem_read(int32_t, address, v1);
            reg_write(intptr_t, rd, v1);
        }

        __function void rv_lbu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            uint8_t v1 = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            address += (intptr_t) (int16_t) imm_11_0;

            mem_read(uint8_t, address, v1);
            reg_write(uintptr_t, rd, v1);
        }

        __function void rv_lhu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            uint16_t v1 = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            address += (intptr_t) (int16_t) imm_11_0;

            mem_read(uint16_t, address, v1);
            reg_write(uintptr_t, rd, v1);
        }

        __function void rv_lwu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            uint64_t v1 = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            address += (intptr_t) (int16_t) imm_11_0;

            mem_read(uint64_t, address, v1);
            reg_write(uint64_t, rd, v1);
        }

        __function void rv_ld(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            int64_t v1 = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            address += (intptr_t) (int16_t) imm_11_0;

            mem_read(int64_t, address, v1);
            reg_write(int64_t, rd, v1);
        }

        __function void rv_flq(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
        }

        __function void rv_fence(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
        }

        __function void rv_fence_i(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
        }

        __function void rv_jalr(uint16_t imm_11_0, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uintptr_t retval = 0;

            reg_read(uintptr_t, rs1, address);
            address += (intptr_t) (int16_t) imm_11_0;
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
    };

    // r-type
    namespace rtype {
        __function void rv_scw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t word = 0;

            mem_read(uintptr_t, rs1, address);
            reg_read(int32_t, rs2, word);

            if (rvm64::memory::vm_check_load_rsv(address)) {
                mem_write(int32_t, address, word);
                reg_write(int32_t, rd, 0);

                rvm64::memory::vm_clear_load_rsv();
            } else {
                reg_write(int32_t, rd, 1);
            }
        }

        __function void rv_scd(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int64_t value = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            reg_read(int64_t, rs2, value);

            if (!rvm64::memory::vm_check_load_rsv(address)) {
                reg_write(int64_t, rd, 1);
                return;
            }

            mem_write(int64_t, address, value);
            rvm64::memory::vm_clear_load_rsv();
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
            __int128 result = (__int128) (int64_t) v1 * (__int128) (int64_t) v2;
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
            __int128 result = (__int128) (int64_t) v1 * (__uint128_t) (uint64_t) v2;
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

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);

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

        __function void rv_amoswap_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int64_t, address, v1);
                reg_write(int64_t, rd, v1);

                reg_read(int64_t, rs2, v2);
                mem_write(int64_t, address, v2);

                rvm64::memory::vm_clear_load_rsv();
            }
        }

        __function void rv_amoadd_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int64_t, address, v1);
                reg_read(int64_t, rs2, v2);

                mem_write(int64_t, address, (v1 + v2));
                reg_write(int64_t, rd, v1);

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoxor_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 ^ v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoand_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 & v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoor_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 | v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomin_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomax_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amominu_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomaxu_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoswap_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int32_t, address, v1);
                reg_write(int32_t, rd, v1);

                reg_read(int32_t, rs2, v2);
                mem_write(int32_t, address, v2);

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoadd_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 + v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoxor_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 ^ v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoand_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 & v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoor_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 | v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomin_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomax_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amominu_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint32_t, rs2, v2);
                mem_read(uint32_t, address, v1);

                reg_write(uint32_t, rd, v1);
                mem_write(uint32_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomaxu_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint32_t, rs2, v2);
                mem_read(uint32_t, address, v1);

                reg_write(uint32_t, rd, v1);
                mem_write(uint32_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_fsqrt_d() {}
        __function void rv_fmv_d_x() {}
        __function void rv_fcvt_s_d() {}
        __function void rv_fcvt_s_q() {}
        __function void rv_fcvt_d_s() {}
        __function void rv_fcvt_d_q() {}
        __function void rv_fcvt_w_d() {}
        __function void rv_fcvt_wu_d() {}
        __function void rv_fcvt_l_d() {}
        __function void rv_fcvt_lu_d() {}
        __function void rv_fcvt_d_w() {}
        __function void rv_fcvt_d_wu() {}
        __function void rv_fcvt_d_l() {}
        __function void rv_fcvt_d_lu() {}
    };

    namespace stype {
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
    };

    namespace btype {
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
    };

    namespace utype {
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
    };

    namespace jtype {
        __function void rv_jal(uint32_t imm_20_10_1_11_19_12, uint8_t rd) {
            /*
                           Description
                           Jump to address and place return address in rd.

                           Implementation
                           x[rd] = pc+4; pc += sext(offset)
                           */
        }
    };

};
