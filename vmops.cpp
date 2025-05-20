#include <compare>

#include "monodef.hpp"
#include "vmmem.h"
#include "vmmain.h"
#include "vmctx.h"

namespace rvm64::operation {
    bool is_nan(double x) {
        union {
            double d = 0LL;
            uint64_t u = 0LL;
        } converter;

        converter.d = x;

        uint64_t exponent = converter.u & EXPONENT_MASK;
        uint64_t fraction = converter.u & FRACTION_MASK;

        return (exponent == EXPONENT_MASK) && (fraction != 0);
    }

    // i-type
    namespace itype {
        __function void rv_lrw() {
            uint8_t _rd = 0, _rs1 = 0;
            uintptr_t address = 0;
            int32_t value = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(uintptr_t, address, _rs1);
            mem_read(int32_t, value, address);
            reg_write(int32_t, _rd, value);

            rvm64::memory::vm_set_load_rsv(address);
        }

        __function void rv_lrd() {
            uint8_t _rd = 0, _rs1 = 0;
            uintptr_t address = 0;
            int64_t value = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            reg_read(uintptr_t, address, _rs1);

            rvm64::memory::vm_set_load_rsv(address);

            mem_read(int64_t, value, address);
            reg_write(int64_t, _rd, value);
        }

        __function void rv_fmv_d_x() {
            uint8_t _rd = 0, _rs1 = 0;
            int64_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(int64_t, v1, _rs1);
            reg_write(int64_t, _rd, v1);
        }

        __function void rv_fcvt_s_d() {
            uint8_t _rd = 0, _rs1 = 0;
            float v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(double, v1, _rs1);
            reg_write(float, _rd, v1);
        }

        __function void rv_fcvt_d_s() {
            uint8_t _rd = 0, _rs1 = 0;
            double v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(float, v1, _rs1);
            reg_write(double, _rd, v1);
        }

        __function void rv_fcvt_w_d() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(double, v1, _rs1);
            reg_write(int32_t, _rd, v1);
        }

        __function void rv_fcvt_wu_d() {
            uint8_t _rd = 0, _rs1 = 0;
            uint32_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(double, v1, _rs1);
            reg_write(uint32_t, _rd, v1);
        }

        __function void rv_fcvt_d_w() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(int32_t, v1, _rs1);
            reg_write(double, _rd, v1);
        }

        __function void rv_fcvt_d_wu() {
            uint8_t _rd = 0, _rs1 = 0;
            uint32_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(uint32_t, v1, _rs1);
            reg_write(double, _rd, v1);
        }

        // NOTE: maybe an I_TYPE
        __function void rv_fclass_d() {
            uint8_t _rd = 0, _rs1 = 0;
            double v1 = 0;

            union {
                double d = 0LL;
                int64_t u = 0LL;
            } converter;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);

            reg_read(double, v1, _rs1);
            converter.d = v1;

            const uint64_t exponent = (converter.u >> 52) & 0x7FF;
            const uint64_t fraction = converter.u & 0xFFFFFFFFFFFFF;
            const uint64_t sign = converter.u >> 63;

            if (exponent == 0x7FF) {
                if (fraction == 0) {
                    if (sign == 0) {
                        reg_write(int, _rd, 0x7); // +inf
                    } else {
                        reg_write(int, _rd, 0x0); // -inf
                    }
                } else {
                    if (fraction & (1LL << 51)) {
                        reg_write(int, _rd, 0x8); // quiet NaN
                    } else {
                        reg_write(int, _rd, 0x9); // signaling NaN
                    }
                }
            } else if (exponent == 0) {
                if (fraction == 0) {
                    if (sign == 0) {
                        reg_write(int, _rd, 0x4); // +0
                    } else {
                        reg_write(int, _rd, 0x3); // -0
                    }
                } else {
                    if (sign == 0) {
                        reg_write(int, _rd, 0x5); // +subnormal
                    } else {
                        reg_write(int, _rd, 0x2); // -subnormal
                    }
                }
            } else {
                if (sign == 0) {
                    reg_write(int, _rd, 0x6); // +normal
                } else {
                    reg_write(int, _rd, 0x1); // -normal
                }
            }
        }

        // NOTE: immediates are always signed unless there's a bitwise operation
        __function void rv_addi() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0, v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(int32_t, v1, _rs1);
            reg_write(int32_t, _rd, (v1 + _imm));
        }

        __function void rv_slti() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0, v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(int32_t, v1, _rs1);
            reg_write(int32_t, _rd, ((v1 < _imm) ? 1 : 0));
        }

        __function void rv_sltiu() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0, v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uint32_t, v1, _rs1);
            reg_write(uint32_t, _rd, ((v1 < (uint32_t)_imm) ? 1 : 0));
        }

        __function void rv_xori() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0, v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(int32_t, v1, _rs1);
            reg_write(int32_t, _rd, (v1 ^ _imm));
        }

        __function void rv_ori() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0, v1 = 0;

            reg_read(int32_t, v1, _rs1);
            reg_write(int32_t, _rd, (v1 | _imm));
        }

        __function void rv_andi() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0, v1 = 0;

            reg_read(int32_t, v1, _rs1);
            reg_write(int32_t, _rd, (v1 & _imm));
        }

        __function void rv_slli() {
            uint8_t _rd = 0, _rs1 = 0;
            uint32_t shamt = 0, v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint32_t, shamt, shamt);

            reg_read(uint32_t, _rs1, v1);
            reg_write(uint32_t, _rd, (v1 << (shamt & 0x1F)));
        }

        __function void rv_srli() {
            uint8_t _rd = 0, _rs1 = 0;
            intptr_t v1 = 0;
            uint32_t shamt = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint32_t, shamt, shamt);

            reg_read(uint32_t, v1, _rs1);
            reg_write(uint32_t, _rd, v1 >> (shamt & 0x1F));
        }

        __function void rv_srai() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t v1 = 0;
            uint32_t shamt = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint32_t, shamt, imm);

            reg_read(int32_t, v1, _rs1);
            reg_write(int32_t, _rd, v1 >> (shamt & 0x1F));
        }

        __function void rv_addiw() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t v1 = 0;
            int32_t imm = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, imm, imm);

            reg_read(int32_t, v1, _rs1);
            reg_write(int32_t, _rd, v1 + imm);
        }

        __function void rv_slliw() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t v1 = 0;
            uint32_t shamt = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint32_t, shamt, imm);

            reg_read(int32_t, v1, _rs1);
            if ((shamt >> 5) != 0) {
                vmcs->halt = 1;
                vmcs->reason = illegal_op;
                return;
            }

            reg_write(int32_t, _rd, v1 << (shamt & 0x1F));
        }

        __function void rv_srliw() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t v1 = 0;
            uint32_t shamt = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint32_t, shamt, imm);

            reg_read(int32_t, v1, _rs1);
            if ((shamt >> 5) != 0) {
                vmcs->halt = 1;
                vmcs->reason = illegal_op;
                return;
            }

            reg_write(int32_t, _rd, v1 >> (shamt & 0x1F));
        }

        __function void rv_sraiw() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t v1 = 0;
            uint32_t shamt = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint32_t, shamt, imm);

            reg_read(int32_t, v1, _rs1);
            if ((shamt >> 5) != 0) {
                vmcs->halt = 1;
                vmcs->reason = illegal_op;
                return;
            }

            reg_write(int32_t, _rd, v1 >> (shamt & 0x1F));
        }

        __function void rv_lb() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;
            int8_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;

            mem_read(int8_t, v1, address);
            reg_write(int8_t, _rd, v1); // writing data to memory does not care about types
        }

        __function void rv_lh() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;
            int16_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;

            mem_read(int16_t, v1, address);
            reg_write(int16_t, _rd, v1);
        }

        __function void rv_lw() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;
            int32_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;

            mem_read(int32_t, v1, address);
            reg_write(int32_t, _rd, v1);
        }

        __function void rv_lbu() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;
            uint8_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;

            mem_read(uint8_t, v1, address);
            reg_write(uint8_t, _rd, v1);
        }

        __function void rv_lhu() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;
            uint16_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;

            mem_read(uint16_t, v1, address);
            reg_write(uint16_t, _rd, v1);
        }

        __function void rv_lwu() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;
            uint32_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;

            mem_read(uint32_t, v1, address);
            reg_write(uint32_t, _rd, v1);
        }

        __function void rv_ld() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;
            int64_t v1 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;

            mem_read(int64_t, v1, address);
            reg_write(int64_t, _rd, v1);
        }

        __function void rv_flq() {
            // TODO
        }

        __function void rv_fence() {
            // TODO
        }

        __function void rv_fence_i() {
            // TODO
        }

        __function void rv_jalr() {
            uint8_t _rd = 0, _rs1 = 0;
            int32_t _imm = 0;
            uintptr_t address = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(int32_t, _imm, imm);

            reg_read(uintptr_t, address, _rs1);
            address += (intptr_t)_imm;
            address &= ~1;

            reg_write(uintptr_t, _rd, vmcs->pc + 4);
            vmcs->pc = address;
        }

        __function void rv_ecall() {
            // TODO
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

        __function void rv_ebreak() {
            /*
                           Description
                           Used by debuggers to cause control to be transferred back to a debugging environment.
                           It generates a breakpoint exception and performs no other operation.

                           Implementation
                           RaiseException(Breakpoint)
                           */
            __debugbreak();
        }

        __function void rv_csrrw() {
            /*
                           Description
                           Atomically swaps values in the CSRs and integer registers.
                           CSRRW reads the old value of the CSR, zero-extends the value to XLEN bits, then writes it to integer register _rd.
                           The initial value in _rs1 is written to the CSR.
                           If _rd=x0, then the instruction shall not read the CSR and shall not cause any of the side effects that might occur on a CSR read.

                           Implementation
                           t = CSRs[csr]; CSRs[csr] = x[_rs1]; x[_rd] = t
                           */
        }

        __function void rv_csrrs() {
            /*
                           Description
                           Reads the value of the CSR, zero-extends the value to XLEN bits, and writes it to integer register _rd.
                           The initial value in integer register _rs1 is treated as a bit mask that specifies bit positions to be set in the CSR.
                           Any bit that is high in _rs1 will cause the corresponding bit to be set in the CSR, if that CSR bit is writable.
                           Other bits in the CSR are unaffected (though CSRs might have side effects when written).

                           Implementation
                           t = CSRs[csr]; CSRs[csr] = t | x[_rs1]; x[_rd] = t
                           */
        }

        __function void rv_csrrc() {
            /*
                           Description
                           Reads the value of the CSR, zero-extends the value to XLEN bits, and writes it to integer register _rd.
                           The initial value in integer register _rs1 is treated as a bit mask that specifies bit positions to be cleared in the CSR.
                           Any bit that is high in _rs1 will cause the corresponding bit to be cleared in the CSR, if that CSR bit is writable.
                           Other bits in the CSR are unaffected.

                           Implementation
                           t = CSRs[csr]; CSRs[csr] = t &~x[_rs1]; x[_rd] = t
                           */
        }

        __function void rv_csrrwi() {
            /*
                           Description
                           Update the CSR using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the _rs1 field.

                           Implementation
                           x[_rd] = CSRs[csr]; CSRs[csr] = zimm
                           */
        }

        __function void rv_csrrsi() {
            /*
                           Description
                           Set CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the _rs1 field.

                           Implementation
                           t = CSRs[csr]; CSRs[csr] = t | zimm; x[_rd] = t
                           */
        }

        __function void rv_csrrci() {
            /*
                           Description
                           Clear CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the _rs1 field.

                           Implementation
                           t = CSRs[csr]; CSRs[csr] = t &~zimm; x[_rd] = t
                           */
        }
    };

    // r-type
    namespace rtype {
        __function void rv_scw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t address = 0;
            int32_t value = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            mem_read(uintptr_t, address, _rs1);
            reg_read(int32_t, value, _rs2);

            if (rvm64::memory::vm_check_load_rsv(address)) {
                mem_write(int32_t, address, value);
                reg_write(int32_t, _rd, 0);
                rvm64::memory::vm_clear_load_rsv();
            } else {
                reg_write(int32_t, _rd, 1);
            }
        }

        __function void rv_scd() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int64_t value = 0;
            uintptr_t address = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, address, _rs1);
            reg_read(int64_t, value, _rs2);

            if (!rvm64::memory::vm_check_load_rsv(address)) {
                reg_write(int64_t, _rd, 1);
                return;
            }

            mem_write(int64_t, address, value);
            rvm64::memory::vm_clear_load_rsv();
        }

        __function void rv_fadd_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            float v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(float, v1, _rs1);
            reg_read(float, v2, _rs2);
            reg_write(float, _rd, (v1 + v2));
        }

        __function void rv_fsub_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            float v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(float, v1, _rs1);
            reg_read(float, v2, _rs2);
            reg_write(float, _rd, (v1 - v2));
        }

        __function void rv_fmul_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            float v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(float, v1, _rs1);
            reg_read(float, v2, _rs2);
            reg_write(float, _rd, (v1 * v2));
        }

        __function void rv_fdiv_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            float v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(float, v1, _rs1);
            reg_read(float, v2, _rs2);
            reg_write(float, _rd, (v1 / v2));
        }


        __function void rv_fsgnj_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int64_t, v1, _rs1);
            reg_read(int64_t, v2, _rs2);

            int64_t s2 = (v2 >> 63) & 1;

            v1 &= ~(1LL << 63);
            v1 |= (s2 << 63);

            reg_write(int64_t, _rd, v1);
        }

        __function void rv_fsgnjn_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int64_t, v1, _rs1);
            reg_read(int64_t, v2, _rs2);

            int64_t s2 = ((v2 >> 63) & 1) ^ 1;

            v1 &= ~(1LL << 63);
            v1 |= (s2 << 63);

            reg_write(int64_t, _rd, v1);
        }

        __function void rv_fsgnjx_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int64_t, v1, _rs1);
            reg_read(int64_t, v2, _rs2);

            int64_t s1 = (v1 >> 63) & 1;
            int64_t s2 = (v2 >> 63) & 1;

            v1 &= ~(1LL << 63);
            v1 |= ((s1 ^ s2) << 63);

            reg_write(int64_t, _rd, v1);
        }

        __function void rv_fmin_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            double v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(double, v1, _rs1);
            reg_read(double, v2, _rs2);

            if (is_nan(v1)) {
                reg_write(double, _rd, v2);
            } else if (is_nan(v2)) {
                reg_write(double, _rd, v1);
            } else {
                reg_write(double, _rd, (v1 > v2) ? v2 : v1);
            }
        }

        __function void rv_fmax_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            double v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(double, v1, _rs1);
            reg_read(double, v2, _rs2);

            if (is_nan(v1)) {
                reg_write(double, _rd, v2);
            } else if (is_nan(v2)) {
                reg_write(double, _rd, v1);
            } else {
                reg_write(double, _rd, (v1 > v2) ? v1 : v2);
            }
        }

        __function void rv_feq_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            double v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(double, v1, _rs1);
            reg_read(double, v2, _rs2);

            if (is_nan(v1) || is_nan(v2)) {
                reg_write(bool, _rd, false);
                return;
            }

            reg_write(bool, _rd, (v1 == v2));
        }

        __function void rv_flt_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            double v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(double, v1, _rs1);
            reg_read(double, v2, _rs2);

            if (is_nan(v1) || is_nan(v2)) {
                reg_write(bool, _rd, false);
                return;
            }

            reg_write(bool, _rd, (v1 < v2));
        }

        __function void rv_fle_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            double v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(double, v1, _rs1);
            reg_read(double, v2, _rs2);

            if (is_nan(v1) || is_nan(v2)) {
                reg_write(bool, _rd, false);
                return;
            }

            reg_write(bool, _rd, (v1 <= v2));
        }

        __function void rv_addw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int64_t, _rd, (int64_t)(v1 + v2));
        }

        __function void rv_subw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int64_t, _rd, (int64_t)(v1 - v2));
        }

        __function void rv_mulw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int64_t, _rd, (int64_t)(v1 * v2));
        }

        __function void rv_srlw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0;
            uint32_t v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(uint32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 >> (v2 & 0x1F)));
        }

        __function void rv_sraw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0;
            uint32_t v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(uint32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 >> (v2 & 0x1F)));
        }

        __function void rv_divuw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uint32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uint32_t, v1, _rs1);
            reg_read(uint32_t, v2, _rs2);
            reg_write(uint32_t, _rd, (v1 / v2));
        }

        __function void rv_sllw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0;
            uint32_t v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 << (v2 & 0x1F)));
        }

        __function void rv_divw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 / v2));
        }

        __function void rv_remw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 % v2));
        }

        __function void rv_remuw() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uint32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uint32_t, v1, _rs1);
            reg_read(uint32_t, v2, _rs2);
            reg_write(uint32_t, _rd, (v1 % v2));
        }

        __function void rv_add() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 + v2));
        }

        __function void rv_sub() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 - v2));
        }

        __function void rv_mul() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, v1, _rs1);
            reg_read(int32_t, v2, _rs2);
            reg_write(int32_t, _rd, (v1 * v2));
        }

        __function void rv_sll() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            int32_t v1 = 0;
            uint32_t v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(int32_t, _rs1, v1);
            reg_read(int32_t, _rs2, v2);
            reg_write(int32_t, _rd, (v1 << (v2 & 0x1F)));
        }

        __function void rv_mulh() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            intptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(intptr_t, v1, _rs1);
            reg_read(intptr_t, v2, _rs2);

#if UINTPTR_MAX == 0xFFFFFFFF
    int64_t result = (int64_t)v1 * (int64_t)v2;
    reg_write(int32_t, _rd, (result >> 32));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
            __int128 result = (__int128)v1 * (__int128)v2;
            reg_write(int64_t, _rd, (result >> 64));

#endif
        }

        __function void rv_slt() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            intptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(intptr_t, v1, _rs1);
            reg_read(intptr_t, v2, _rs2);
            reg_write(intptr_t, _rd, ((v1 < v2) ? 1 : 0));
        }

        __function void rv_mulhsu() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            intptr_t v1 = 0;
            uintptr_t v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(intptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);

#if UINTPTR_MAX == 0xFFFFFFFF
    int64_t result = (int64_t)(int32_t)v1 * (uint64_t)(uint32_t)v2;
    reg_write(int32_t, _rd, (result >> 32));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
            __int128 result = (__int128) (int64_t) v1 * (__uint128_t) (uint64_t) v2;
            reg_write(int64_t, _rd, (result >> 64));

#endif
        }

        __function void rv_sltu() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);
            reg_write(uintptr_t, _rd, ((v1 < v2) ? 1 : 0));
        }

        __function void rv_mulhu() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);

            uintptr_t result = v1 * v2;
#if UINTPTR_MAX == 0xFFFFFFFF
    reg_write(uintptr_t, _rd, (result >> 16));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
            reg_write(uintptr_t, _rd, (result >> 32));
#endif
        }

        __function void rv_xor() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);
            reg_write(uintptr_t, _rd, (v1 ^ v2));
        }

        __function void rv_div() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);
            reg_write(uintptr_t, _rd, (v1 / v2));
        }

        __function void rv_srl() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0;
            uint32_t v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uint32_t, v2, _rs2);
            reg_write(uintptr_t, _rd, (v1 >> (v2 & 0x1F)));
        }

        __function void rv_sra() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            intptr_t v1 = 0;
            uint32_t v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(intptr_t, v1, _rs1);
            reg_read(uint32_t, v2, _rs2);

            reg_write(intptr_t, _rd, (v1 >> (v2 & 0x1F)));
        }

        __function void rv_divu() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);

            if (v2 == 0) {
                reg_write(uintptr_t, _rd, 0);
            } else {
                reg_write(uintptr_t, _rd, (v1 / v2));
            }
        }

        __function void rv_or() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            intptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(intptr_t, _rs1, v1);
            reg_read(intptr_t, _rs2, v2);
            reg_write(intptr_t, _rd, (v1 | v2));
        }

        __function void rv_rem() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            intptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(intptr_t, v1, _rs1);
            reg_read(intptr_t, v2, _rs2);

            if (v2 == 0) {
                reg_write(intptr_t, _rd, 0);
            } else {
                reg_write(intptr_t, _rd, (v1 % v2));
            }
        }

        __function void rv_and() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);
            reg_write(uintptr_t, _rd, (v1 & v2));
        }

        __function void rv_remu() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, v1, _rs1);
            reg_read(uintptr_t, v2, _rs2);

            if (v2 == 0) {
                reg_write(uintptr_t, _rd, 0);
            } else {
                reg_write(uintptr_t, _rd, (v1 % v2));
            }
        }

        // TODO: consider using system mutex. these "soft" locks can still race in different threads.
        __function void rv_amoswap_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, address, _rs1);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int64_t, v1, address);
                reg_write(int64_t, _rd, v1);

                reg_read(int64_t, v2, _rs2);
                mem_write(int64_t, address, v2);

                rvm64::memory::vm_clear_load_rsv();
            }
        }

        __function void rv_amoadd_d() {
            uint8_t _rd = 0, _rs1 = 0, _rs2 = 0;
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, address, _rs1);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int64_t, v1, address);
                reg_read(int64_t, v2, _rs2);

                mem_write(int64_t, address, (v1 + v2));
                reg_write(int64_t, _rd, v1);

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoxor_d() {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, _rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, _rd, v1);
                mem_write(int64_t, address, (v1 ^ v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoand_d() {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, _rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, _rd, v1);
                mem_write(int64_t, address, (v1 & v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoor_d() {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, _rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, _rd, v1);
                mem_write(int64_t, address, (v1 | v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomin_d() {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, _rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, _rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomax_d() {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, _rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, _rd, v1);
                mem_write(int64_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amominu_d() {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, _rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, _rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomaxu_d() {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, _rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, _rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoswap_w() {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            mem_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int32_t, address, v1);
                reg_write(int32_t, _rd, v1);

                reg_read(int32_t, _rs2, v2);
                mem_write(int32_t, address, v2);

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoadd_w() {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            mem_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, _rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, _rd, v1);
                mem_write(int32_t, address, (v1 + v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoxor_w() {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, _rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, _rd, v1);
                mem_write(int32_t, address, (v1 ^ v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoand_w() {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, _rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, _rd, v1);
                mem_write(int32_t, address, (v1 & v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoor_w() {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, _rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, _rd, v1);
                mem_write(int32_t, address, (v1 | v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomin_w() {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, _rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, _rd, v1);
                mem_write(int32_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomax_w() {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, _rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, _rd, v1);
                mem_write(int32_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amominu_w() {
            uintptr_t address = 0;
            uint32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint32_t, _rs2, v2);
                mem_read(uint32_t, address, v1);

                reg_write(uint32_t, _rd, v1);
                mem_write(uint32_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomaxu_w() {
            uintptr_t address = 0;
            uint32_t v1 = 0, v2 = 0;

            scr_read(uint8_t, _rd, rd);
            scr_read(uint8_t, _rs1, rs1);
            scr_read(uint8_t, _rs2, rs2);

            reg_read(uintptr_t, _rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint32_t, _rs2, v2);
                mem_read(uint32_t, address, v1);

                reg_write(uint32_t, _rd, v1);
                mem_write(uint32_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_fsqrt_d() {
            /*
                           Description
                           Take the branch if registers _rs1 and _rs2 are equal.

                           Implementation
                           if (x[_rs1] == x[_rs2]) pc += sext(offset)
                           */
        }

        __function void rv_bne() {
            /*
                           Description
                           Take the branch if registers _rs1 and _rs2 are not equal.

                           Implementation
                           if (x[_rs1] != x[_rs2]) pc += sext(offset)
                           */
        }

        __function void rv_blt() {
            /*
                           Description
                           Take the branch if registers _rs1 is less than _rs2, using signed comparison.

                           Implementation
                           if (x[_rs1] <s x[_rs2]) pc += sext(offset)
                           */
        }

        __function void rv_bge() {
            /*
                           Description
                           Take the branch if registers _rs1 is greater than or equal to _rs2, using signed comparison.

                           Implementation
                           if (x[_rs1] >=s x[_rs2]) pc += sext(offset)
                           */
        }

        __function void rv_bltu() {
            /*
                           Description
                           Take the branch if registers _rs1 is less than _rs2, using unsigned comparison.

                           Implementation
                           if (x[_rs1] <u x[_rs2]) pc += sext(offset)
                           */
        }

        __function void rv_bgeu() {
            /*
                           Description
                           Take the branch if registers _rs1 is greater than or equal to _rs2, using unsigned comparison.

                           Implementation
                           if (x[_rs1] >=u x[_rs2]) pc += sext(offset)
                           */
        }
    };

    namespace utype {
        __function void rv_lui() {
            /*
                           Description
                           Build 32-bit constants and uses the U-type format. LUI places the U-immediate value in the top 20 bits of the destination register _rd,
                           filling in the lowest 12 bits with zeros.

                           Implementation
                           x[_rd] = sext(immediate[31:12] << 12)
                           */
        }

        __function void rv_auipc() {
            /*
                           Description
                           Build pc-relative addresses and uses the U-type format. AUIPC forms a 32-bit offset from the 20-bit U-immediate,
                           filling in the lowest 12 bits with zeros, adds this offset to the pc, then places the result in register _rd.

                           Implementation
                           x[_rd] = pc + sext(immediate[31:12] << 12)
                           */
        }
    };

    namespace jtype {
        __function void rv_jal() {
            /*
                           Description
                           Jump to address and place return address in _rd.

                           Implementation
                           x[_rd] = pc+4; pc += sext()
                           */
        }
    };
};
