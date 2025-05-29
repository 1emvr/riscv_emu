#ifndef VMOPS_H
#define VMOPS_H
#include <compare>

#include "vmmain.hpp"
#include "vmcrypt.hpp"
#include "vmctx.hpp"
#include "vmatom.hpp"

namespace rvm64::operation {
	union {
		double d;
		uint64_t u;
	} converter;

	__vmcall bool is_nan(double x) {
		converter.d = x;

		uint64_t exponent = (converter.u & EXPONENT_MASK) >> 52;
		uint64_t fraction = converter.u & FRACTION_MASK;

		return (exponent == 0x7FF) && (fraction != 0);
	}

	namespace itype {
		__vmcall void rv_lrw() {
			uint8_t _rd = 0, _rs1 = 0; uintptr_t address = 0; int32_t value = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(uintptr_t, address, _rs1);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int32_t, value, address);
			reg_write(int32_t, _rd, value);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_lrd() {
			uint8_t _rd = 0, _rs1 = 0; uintptr_t address = 0; int64_t value = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(uintptr_t, address, _rs1);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, value, address);
			reg_write(int64_t, _rd, value);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_fmv_d_x() {
			uint8_t _rd = 0, _rs1 = 0; int64_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(int64_t, v1, _rs1);
			reg_write(int64_t, _rd, v1);
		}

		__vmcall void rv_fcvt_s_d() {
			uint8_t _rd = 0, _rs1 = 0; float v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(double, v1, _rs1);
			reg_write(float, _rd, v1);
		}

		__vmcall void rv_fcvt_d_s() {
			uint8_t _rd = 0, _rs1 = 0; double v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(float, v1, _rs1);
			reg_write(double, _rd, v1);
		}

		__vmcall void rv_fcvt_w_d() {
			uint8_t _rd = 0, _rs1 = 0; int32_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(double, v1, _rs1);
			reg_write(int32_t, _rd, v1);
		}

		__vmcall void rv_fcvt_wu_d() {
			uint8_t _rd = 0, _rs1 = 0; uint32_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(double, v1, _rs1);
			reg_write(uint32_t, _rd, v1);
		}

		__vmcall void rv_fcvt_d_w() {
			uint8_t _rd = 0, _rs1 = 0; int32_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(int32_t, v1, _rs1);
			reg_write(double, _rd, v1);
		}

		__vmcall void rv_fcvt_d_wu() {
			uint8_t _rd = 0, _rs1 = 0; uint32_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);

			reg_read(uint32_t, v1, _rs1);
			reg_write(double, _rd, v1);
		}

		// NOTE: maybe not even real...
		__vmcall void rv_fclass_d() {
			uint8_t _rd = 0, _rs1 = 0; double v1 = 0;

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
		__vmcall void rv_addi() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0, v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(int32_t, v1, _rs1);
			reg_write(int32_t, _rd, (v1 + _imm));
		}

		__vmcall void rv_slti() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0, v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(int32_t, v1, _rs1);
			reg_write(int32_t, _rd, ((v1 < _imm) ? 1 : 0));
		}

		__vmcall void rv_sltiu() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0, v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uint32_t, v1, _rs1);
			reg_write(uint32_t, _rd, ((v1 < (uint32_t)_imm) ? 1 : 0));
		}

		__vmcall void rv_xori() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0, v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(int32_t, v1, _rs1);
			reg_write(int32_t, _rd, (v1 ^ _imm));
		}

		__vmcall void rv_ori() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0, v1 = 0;

			reg_read(int32_t, v1, _rs1);
			reg_write(int32_t, _rd, (v1 | _imm));
		}

		__vmcall void rv_andi() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0, v1 = 0;

			reg_read(int32_t, v1, _rs1);
			reg_write(int32_t, _rd, (v1 & _imm));
		}

		__vmcall void rv_slli() {
			uint8_t _rd = 0, _rs1 = 0; uint32_t shamt = 0, v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint32_t, shamt, shamt);

			reg_read(uint32_t, _rs1, v1);
			reg_write(uint32_t, _rd, (v1 << (shamt & 0x1F)));
		}

		__vmcall void rv_srli() {
			uint8_t _rd = 0, _rs1 = 0; intptr_t v1 = 0; uint32_t shamt = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint32_t, shamt, shamt);

			reg_read(uint32_t, v1, _rs1);
			reg_write(uint32_t, _rd, v1 >> (shamt & 0x1F));
		}

		__vmcall void rv_srai() {
			uint8_t _rd = 0, _rs1 = 0; int32_t v1 = 0; uint32_t shamt = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint32_t, shamt, imm);

			reg_read(int32_t, v1, _rs1);
			reg_write(int32_t, _rd, v1 >> (shamt & 0x1F));
		}

		__vmcall void rv_addiw() {
			uint8_t _rd = 0, _rs1 = 0; int32_t v1 = 0; int32_t imm = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, imm, imm);

			reg_read(int32_t, v1, _rs1);
			reg_write(int32_t, _rd, v1 + imm);
		}

		__vmcall void rv_slliw() {
			uint8_t _rd = 0, _rs1 = 0; int32_t v1 = 0; uint32_t shamt = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint32_t, shamt, imm);

			reg_read(int32_t, v1, _rs1);
			if ((shamt >> 5) != 0) {
				vmcs->halt = 1;
				vmcs->reason = vm_illegal_op;
				return;
			}

			reg_write(int32_t, _rd, v1 << (shamt & 0x1F));
		}

		__vmcall void rv_srliw() {
			uint8_t _rd = 0, _rs1 = 0; int32_t v1 = 0; uint32_t shamt = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint32_t, shamt, imm);

			reg_read(int32_t, v1, _rs1);
			if ((shamt >> 5) != 0) {
				vmcs->halt = 1;
				vmcs->reason = vm_illegal_op;
				return;
			}

			reg_write(int32_t, _rd, v1 >> (shamt & 0x1F));
		}

		__vmcall void rv_sraiw() {
			uint8_t _rd = 0, _rs1 = 0; int32_t v1 = 0; uint32_t shamt = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint32_t, shamt, imm);

			reg_read(int32_t, v1, _rs1);

			if ((shamt >> 5) != 0) {
				vmcs->halt = 1;
				vmcs->reason = vm_illegal_op;
				return;
			}

			reg_write(int32_t, _rd, v1 >> (shamt & 0x1F));
		}

		__vmcall void rv_lb() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0; int8_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;

			mem_read(int8_t, v1, address);
			reg_write(int8_t, _rd, v1); // writing data to memory does not care about types
		}

		__vmcall void rv_lh() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0; int16_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;

			mem_read(int16_t, v1, address);
			reg_write(int16_t, _rd, v1);
		}

		__vmcall void rv_lw() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0; int32_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;

			mem_read(int32_t, v1, address);
			reg_write(int32_t, _rd, v1);
		}

		__vmcall void rv_lbu() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0; uint8_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;

			mem_read(uint8_t, v1, address);
			reg_write(uint8_t, _rd, v1);
		}

		__vmcall void rv_lhu() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0; uint16_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;

			mem_read(uint16_t, v1, address);
			reg_write(uint16_t, _rd, v1);
		}

		__vmcall void rv_lwu() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0; uint32_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;

			mem_read(uint32_t, v1, address);
			reg_write(uint32_t, _rd, v1);
		}

		__vmcall void rv_ld() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0; int64_t v1 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;

			mem_read(int64_t, v1, address);
			reg_write(int64_t, _rd, v1);
		}

		__vmcall void rv_flq() {
			// TODO
		}

		// NOTE: fence instructions aren't needed rn.
		__vmcall void rv_fence() {
			// TODO
		}

		__vmcall void rv_fence_i() {
			// TODO
		}

		__vmcall void rv_jalr() {
			uint8_t _rd = 0, _rs1 = 0; int32_t _imm = 0; uintptr_t address = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			address += (intptr_t)_imm;
			address &= ~((intptr_t)1);

			reg_write(uintptr_t, _rd, vmcs->pc + 4);

			vmcs->pc = address;
			vmcs->step = false;
			vmcs->csr.m_cause = environment_call_native;
		}

		__vmcall void rv_ecall() {
			// TODO: block all system calls.
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
			// rvm64::context::save_vm_context();
			// rvm64::context::decode_syscall();
			// rvm64::context::setup_arguments();
			// rvm64::context::vm_exit();
			// rvm64::context::restore_vm_context();

		}

		__vmcall void rv_ebreak() {
			/*
			   Description
			   Used by debuggers to cause control to be transferred back to a debugging environment.
			   It generates a breakpoint exception and performs no other operation.

			   Implementation
			   RaiseException(Breakpoint)
			   */
			vmcs->csr.m_cause = breakpoint;
			__debugbreak();
		}

		// NOTE: csr operations aren't needed rn.
		__vmcall void rv_csrrw() {
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

		__vmcall void rv_csrrs() {
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

		__vmcall void rv_csrrc() {
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

		__vmcall void rv_csrrwi() {
			/*
			   Description
			   Update the CSR using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the _rs1 field.

			   Implementation
			   x[_rd] = CSRs[csr]; CSRs[csr] = zimm
			   */
		}

		__vmcall void rv_csrrsi() {
			/*
			   Description
			   Set CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the _rs1 field.

			   Implementation
			   t = CSRs[csr]; CSRs[csr] = t | zimm; x[_rd] = t
			   */
		}

		__vmcall void rv_csrrci() {
			/*
			   Description
			   Clear CSR bit using an XLEN-bit value obtained by zero-extending a 5-bit unsigned immediate (uimm[4:0]) field encoded in the _rs1 field.

			   Implementation
			   t = CSRs[csr]; CSRs[csr] = t &~zimm; x[_rd] = t
			   */
		}
	}

	namespace rtype {
		__vmcall void rv_scw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t value = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			mem_read(uintptr_t, address, _rs1);
			reg_read(int32_t, value, _rs2);

			if (rvm64::atom::vm_check_load_rsv(0, address)) {
				rvm64::atom::vm_set_load_rsv(0, address);

				mem_write(int32_t, address, value);
				reg_write(int32_t, _rd, 0);

				rvm64::atom::vm_clear_load_rsv(0);
			} else {
				reg_write(int32_t, _rd, 1);
			}
		}

		__vmcall void rv_scd() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int64_t value = 0; uintptr_t address = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int64_t, value, _rs2);

			if (!rvm64::atom::vm_check_load_rsv(0, address)) {
				rvm64::atom::vm_set_load_rsv(0, address);

				mem_write(int64_t, address, value);
				reg_write(int64_t, _rd, 0);

				rvm64::atom::vm_clear_load_rsv(0);
			} else {
				reg_write(int64_t, _rd, 1);
			}
		}

		__vmcall void rv_fadd_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; float v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(float, v1, _rs1);
			reg_read(float, v2, _rs2);
			reg_write(float, _rd, (v1 + v2));
		}

		__vmcall void rv_fsub_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; float v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(float, v1, _rs1);
			reg_read(float, v2, _rs2);
			reg_write(float, _rd, (v1 - v2));
		}

		__vmcall void rv_fmul_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; float v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(float, v1, _rs1);
			reg_read(float, v2, _rs2);
			reg_write(float, _rd, (v1 * v2));
		}

		__vmcall void rv_fdiv_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; float v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(float, v1, _rs1);
			reg_read(float, v2, _rs2);
			reg_write(float, _rd, (v1 / v2));
		}


		__vmcall void rv_fsgnj_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int64_t v1 = 0, v2 = 0;

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

		__vmcall void rv_fsgnjn_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int64_t v1 = 0, v2 = 0;

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

		__vmcall void rv_fsgnjx_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int64_t v1 = 0, v2 = 0;

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

		__vmcall void rv_fmin_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; double v1 = 0, v2 = 0;

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

		__vmcall void rv_fmax_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; double v1 = 0, v2 = 0;

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

		__vmcall void rv_feq_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; double v1 = 0, v2 = 0;

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

		__vmcall void rv_flt_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; double v1 = 0, v2 = 0;

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

		__vmcall void rv_fle_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; double v1 = 0, v2 = 0;

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

		__vmcall void rv_addw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int64_t, _rd, (int64_t)(v1 + v2));
		}

		__vmcall void rv_subw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int64_t, _rd, (int64_t)(v1 - v2));
		}

		__vmcall void rv_mulw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int64_t, _rd, (int64_t)(v1 * v2));
		}

		__vmcall void rv_srlw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0; uint32_t v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 >> (v2 & 0x1F)));
		}

		__vmcall void rv_sraw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0; uint32_t v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 >> (v2 & 0x1F)));
		}

		__vmcall void rv_divuw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uint32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uint32_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);
			reg_write(uint32_t, _rd, (v1 / v2));
		}

		__vmcall void rv_sllw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0; uint32_t v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 << (v2 & 0x1F)));
		}

		__vmcall void rv_divw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 / v2));
		}

		__vmcall void rv_remw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 % v2));
		}

		__vmcall void rv_remuw() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uint32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uint32_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);
			reg_write(uint32_t, _rd, (v1 % v2));
		}

		__vmcall void rv_add() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 + v2));
		}

		__vmcall void rv_sub() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 - v2));
		}

		__vmcall void rv_mul() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);
			reg_write(int32_t, _rd, (v1 * v2));
		}

		__vmcall void rv_sll() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; int32_t v1 = 0; uint32_t v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(int32_t, _rs1, v1);
			reg_read(int32_t, _rs2, v2);
			reg_write(int32_t, _rd, (v1 << (v2 & 0x1F)));
		}

		__vmcall void rv_mulh() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; intptr_t v1 = 0, v2 = 0;
			// TODO: CHECK THIS

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

		__vmcall void rv_slt() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; intptr_t v1 = 0, v2 = 0;
			// TODO: CHECK THIS

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(intptr_t, v1, _rs1);
			reg_read(intptr_t, v2, _rs2);
			reg_write(intptr_t, _rd, ((v1 < v2) ? 1 : 0));
		}

		__vmcall void rv_mulhsu() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; intptr_t v1 = 0; uintptr_t v2 = 0;
			// TODO: CHECK THIS

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

		__vmcall void rv_sltu() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, v1, _rs1);
			reg_read(uintptr_t, v2, _rs2);
			reg_write(uintptr_t, _rd, ((v1 < v2) ? 1 : 0));
		}

		__vmcall void rv_mulhu() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0, v2 = 0;
			// TODO: CHECK THIS

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

		__vmcall void rv_xor() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, v1, _rs1);
			reg_read(uintptr_t, v2, _rs2);
			reg_write(uintptr_t, _rd, (v1 ^ v2));
		}

		__vmcall void rv_div() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, v1, _rs1);
			reg_read(uintptr_t, v2, _rs2);
			reg_write(uintptr_t, _rd, (v1 / v2));
		}

		__vmcall void rv_srl() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0; uint32_t v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);
			reg_write(uintptr_t, _rd, (v1 >> (v2 & 0x1F)));
		}

		__vmcall void rv_sra() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; intptr_t v1 = 0; uint32_t v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(intptr_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);

			reg_write(intptr_t, _rd, (v1 >> (v2 & 0x1F)));
		}

		__vmcall void rv_divu() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0, v2 = 0;

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

		__vmcall void rv_or() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; intptr_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(intptr_t, _rs1, v1);
			reg_read(intptr_t, _rs2, v2);
			reg_write(intptr_t, _rd, (v1 | v2));
		}

		__vmcall void rv_rem() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; intptr_t v1 = 0, v2 = 0;

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

		__vmcall void rv_and() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, v1, _rs1);
			reg_read(uintptr_t, v2, _rs2);
			reg_write(uintptr_t, _rd, (v1 & v2));
		}

		__vmcall void rv_remu() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t v1 = 0, v2 = 0;

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

		__vmcall void rv_amoswap_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, v1, address);
			mem_write(int64_t, address, v2);
			reg_write(int64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoadd_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, v1, address);
			mem_write(int64_t, address, (v1 + v2));
			reg_write(int64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoxor_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, v1, address);
			mem_write(int64_t, address, (v1 ^ v2));
			reg_write(int64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoand_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, v1, address);
			mem_write(int64_t, address, (v1 & v2));
			reg_write(int64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoor_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, v1, address);
			mem_write(int64_t, address, (v1 | v2));
			reg_write(int64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amomin_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, v1, address);
			mem_write(int64_t, address, (v1 < v2 ? v1 : v2));
			reg_write(uint64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amomax_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int64_t, v1, address);
			mem_write(int64_t, address, (v1 < v2 ? v2 : v1));
			reg_write(int64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amominu_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; uint64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, _rs1, address);
			reg_read(uint64_t, _rs2, v2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(uint64_t, v1, address);
			mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));
			reg_write(uint64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amomaxu_d() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; uint64_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint64_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(uint64_t, v1, address);
			mem_write(uint64_t, address, (v1 < v2 ? v2 : v1));
			reg_write(uint64_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoswap_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int32_t, v1, address);
			mem_write(int32_t, address, v2);
			reg_write(int32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoadd_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int32_t, v1, address);
			mem_write(int32_t, address, (v1 + v2));
			reg_write(int32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoxor_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int32_t, v1, address);
			mem_write(int32_t, address, (v1 ^ v2));
			reg_write(int32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoand_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);
			
			mem_read(int32_t, v1, address);
			mem_write(int32_t, address, (v1 & v2));
			reg_write(int32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amoor_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int32_t, v1, address);
			mem_write(int32_t, address, (v1 | v2));
			reg_write(int32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amomin_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int32_t, v1, address);
			mem_write(int32_t, address, (v1 < v2 ? v1 : v2));
			reg_write(int32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amomax_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; int32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(int32_t, v1, address);
			mem_write(int32_t, address, (v1 < v2 ? v2 : v1));
			reg_write(int32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amominu_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; uint32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(uint32_t, v1, address);
			mem_write(uint32_t, address, (v1 < v2 ? v1 : v2));
			reg_write(uint32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}

		__vmcall void rv_amomaxu_w() {
			uint8_t _rd = 0, _rs1 = 0, _rs2 = 0; uintptr_t address = 0; uint32_t v1 = 0, v2 = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint32_t, v2, _rs2);

			ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

			mem_read(uint32_t, v1, address);
			mem_write(uint32_t, address, (v1 < v2 ? v2 : v1));
			reg_write(uint32_t, _rd, v1);

			ctx->win32.NtReleaseMutex(vmcs_mutex);
		}
	}

	namespace utype {
		__vmcall void rv_lui() {
			uint8_t _rd = 0; int32_t _imm = 0;

			scr_read(uint32_t, _rd, rd);
			scr_read(int32_t, _imm, imm);
			reg_write(int32_t, _rd, _imm);
		}

		__vmcall void rv_auipc() {
			uint8_t _rd = 0; int32_t _imm = 0; 

			scr_read(uint8_t, _rd, rd);
			scr_read(int32_t, _imm, imm);
			reg_write(int32_t, _rd, (int64_t)vmcs->pc + _imm);
		}
	}

	namespace jtype {
		__vmcall void rv_jal() {
			uint8_t _rd = 0; intptr_t offset = 0;

			scr_read(uint8_t, _rd, rd);
			scr_read(intptr_t, offset, imm);

			reg_write(uintptr_t, _rd, vmcs->pc + 4);
			vmcs->pc += offset;
			// vmcs->step = false;
		}
	}

	namespace btype {
		__vmcall void rv_beq() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0; intptr_t offset = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(intptr_t, offset, imm);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);

			if (v1 == v2) {
				vmcs->pc += offset;
				// vmcs->step = false;
			}
		}

		__vmcall void rv_bne() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0; intptr_t offset = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(intptr_t, offset, imm);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);

			if (v1 != v2) {
				vmcs->pc += offset;
				// vmcs->step = false;
			}
		}

		__vmcall void rv_blt() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0; intptr_t offset = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(intptr_t, offset, imm);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);

			if (v1 < v2) {
				vmcs->pc += offset;
				// vmcs->step = false;
			}
		}

		__vmcall void rv_bge() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t v1 = 0, v2 = 0; intptr_t offset = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(intptr_t, offset, imm);

			reg_read(int32_t, v1, _rs1);
			reg_read(int32_t, v2, _rs2);

			if (v1 >= v2) {
				vmcs->pc += offset;
				// vmcs->step = false;
			}
		}

		__vmcall void rv_bltu() {
			uint8_t _rs1 = 0, _rs2 = 0; uint32_t v1 = 0, v2 = 0; intptr_t offset = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(intptr_t, offset, imm);

			reg_read(uint32_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);

			if (v1 < v2) {
				vmcs->pc += offset;
				// vmcs->step = false;
			}
		}

		__vmcall void rv_bgeu() {
			uint8_t _rs1 = 0, _rs2 = 0; uint32_t v1 = 0, v2 = 0; intptr_t offset = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(intptr_t, offset, imm);

			reg_read(uint32_t, v1, _rs1);
			reg_read(uint32_t, v2, _rs2);

			if (v1 >= v2) {
				vmcs->pc += offset;
				// vmcs->step = false;
			}
		}
	}

	namespace stype {
		__vmcall void rv_sb() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t _imm = 0; uintptr_t address = 0; uint8_t v1 = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(uint8_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			reg_read(int8_t, v1, _rs2);

			address += (intptr_t)_imm;
			mem_write(uint8_t, address, v1);
		}

		__vmcall void rv_sh() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t _imm = 0; uintptr_t address = 0; uint16_t v1 = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(uint8_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint16_t, v1, _rs2);

			address += (intptr_t)_imm;
			mem_write(uint16_t, address, v1);
		}

		__vmcall void rv_sw() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t _imm = 0; uintptr_t address = 0; uint32_t v1 = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(uint8_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint32_t, v1, _rs2);

			address += (intptr_t)_imm;
			mem_write(uint32_t, address, v1);
		}

		__vmcall void rv_sd() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t _imm = 0; uintptr_t address = 0; uint64_t v1 = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(uint8_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint64_t, v1, _rs2);

			address += (intptr_t)_imm;
			mem_write(uint64_t, address, v1);
		}

		__vmcall void rv_fsw() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t _imm = 0; uintptr_t address = 0; float v1 = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			reg_read(float, v1, _rs2);

			address += (intptr_t)_imm;
			mem_write(float, address, v1);
		}

		__vmcall void rv_fsd() {
			uint8_t _rs1 = 0, _rs2 = 0; int32_t _imm = 0; uintptr_t address = 0; double v1 = 0;

			scr_read(uint8_t, _rs1, rs1);
			scr_read(uint8_t, _rs2, rs2);
			scr_read(int32_t, _imm, imm);

			reg_read(uintptr_t, address, _rs1);
			reg_read(uint64_t, v1, _rs2);

			address += (intptr_t)_imm;
			mem_write(double, address, v1);
		}
	}

	namespace r4type {}

	__rdata const uintptr_t __handler[256] = {
		// ITYPE
		crypt::encrypt_ptr((uintptr_t) itype::rv_addi), crypt::encrypt_ptr((uintptr_t) itype::rv_slti),
		crypt::encrypt_ptr((uintptr_t) itype::rv_sltiu), crypt::encrypt_ptr((uintptr_t) itype::rv_xori),
		crypt::encrypt_ptr((uintptr_t) itype::rv_ori), crypt::encrypt_ptr((uintptr_t) itype::rv_andi),
		crypt::encrypt_ptr((uintptr_t) itype::rv_slli), crypt::encrypt_ptr((uintptr_t) itype::rv_srli),
		crypt::encrypt_ptr((uintptr_t) itype::rv_srai), crypt::encrypt_ptr((uintptr_t) itype::rv_addiw),
		crypt::encrypt_ptr((uintptr_t) itype::rv_slliw), crypt::encrypt_ptr((uintptr_t) itype::rv_srliw),
		crypt::encrypt_ptr((uintptr_t) itype::rv_sraiw), crypt::encrypt_ptr((uintptr_t) itype::rv_lb),
		crypt::encrypt_ptr((uintptr_t) itype::rv_lh), crypt::encrypt_ptr((uintptr_t) itype::rv_lw),

		crypt::encrypt_ptr((uintptr_t) itype::rv_lbu), crypt::encrypt_ptr((uintptr_t) itype::rv_lhu),
		crypt::encrypt_ptr((uintptr_t) itype::rv_lwu), crypt::encrypt_ptr((uintptr_t) itype::rv_ld),
		crypt::encrypt_ptr((uintptr_t) itype::rv_flq), crypt::encrypt_ptr((uintptr_t) itype::rv_fence),
		crypt::encrypt_ptr((uintptr_t) itype::rv_fence_i), crypt::encrypt_ptr((uintptr_t) itype::rv_jalr),
		crypt::encrypt_ptr((uintptr_t) itype::rv_ecall), crypt::encrypt_ptr((uintptr_t) itype::rv_ebreak),
		crypt::encrypt_ptr((uintptr_t) itype::rv_csrrw), crypt::encrypt_ptr((uintptr_t) itype::rv_csrrs),
		crypt::encrypt_ptr((uintptr_t) itype::rv_csrrc), crypt::encrypt_ptr((uintptr_t) itype::rv_csrrwi),
		crypt::encrypt_ptr((uintptr_t) itype::rv_csrrsi), crypt::encrypt_ptr((uintptr_t) itype::rv_csrrci),
		crypt::encrypt_ptr((uintptr_t) itype::rv_fclass_d), crypt::encrypt_ptr((uintptr_t) itype::rv_lrw),
		crypt::encrypt_ptr((uintptr_t) itype::rv_lrd), crypt::encrypt_ptr((uintptr_t) itype::rv_fmv_d_x),
		crypt::encrypt_ptr((uintptr_t) itype::rv_fcvt_s_d), crypt::encrypt_ptr((uintptr_t) itype::rv_fcvt_d_s),
		crypt::encrypt_ptr((uintptr_t) itype::rv_fcvt_w_d), crypt::encrypt_ptr((uintptr_t) itype::rv_fcvt_wu_d),
		crypt::encrypt_ptr((uintptr_t) itype::rv_fcvt_d_w), crypt::encrypt_ptr((uintptr_t) itype::rv_fcvt_d_wu),

		// RTYPE
		crypt::encrypt_ptr((uintptr_t) rtype::rv_fadd_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fsub_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_fmul_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fdiv_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_fsqrt_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fsgnj_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_fsgnjn_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fsgnjx_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_fmin_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fmax_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_feq_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_flt_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_fle_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_scw),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amoswap_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amoadd_w),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amoxor_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amoand_w),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amoor_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amomin_w),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amomax_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amominu_w),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amomaxu_w),

		crypt::encrypt_ptr((uintptr_t) rtype::rv_scd),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amoswap_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_amoadd_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amoxor_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_amoand_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amoor_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_amomin_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amomax_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_amominu_d),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_amomaxu_d),

		crypt::encrypt_ptr((uintptr_t) rtype::rv_addw), crypt::encrypt_ptr((uintptr_t) rtype::rv_subw),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_mulw), crypt::encrypt_ptr((uintptr_t) rtype::rv_srlw),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_sraw), crypt::encrypt_ptr((uintptr_t) rtype::rv_divuw),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_sllw), crypt::encrypt_ptr((uintptr_t) rtype::rv_divw),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_remw), crypt::encrypt_ptr((uintptr_t) rtype::rv_remuw),

		crypt::encrypt_ptr((uintptr_t) rtype::rv_add), crypt::encrypt_ptr((uintptr_t) rtype::rv_sub),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_mul), crypt::encrypt_ptr((uintptr_t) rtype::rv_sll),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_mulh), crypt::encrypt_ptr((uintptr_t) rtype::rv_slt),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_mulhsu), crypt::encrypt_ptr((uintptr_t) rtype::rv_sltu),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_mulhu), crypt::encrypt_ptr((uintptr_t) rtype::rv_xor),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_div), crypt::encrypt_ptr((uintptr_t) rtype::rv_srl),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_sra), crypt::encrypt_ptr((uintptr_t) rtype::rv_divu),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_or), crypt::encrypt_ptr((uintptr_t) rtype::rv_rem),
		crypt::encrypt_ptr((uintptr_t) rtype::rv_and), crypt::encrypt_ptr((uintptr_t) rtype::rv_remu),

		// TODO: Finish floating point instructions (probably S and I types) - f, fm, fnm, fcvt - have a few already done.
		// STYPE
		crypt::encrypt_ptr((uintptr_t) stype::rv_sb), crypt::encrypt_ptr((uintptr_t) stype::rv_sh),
		crypt::encrypt_ptr((uintptr_t) stype::rv_sw), crypt::encrypt_ptr((uintptr_t) stype::rv_sd),
		crypt::encrypt_ptr((uintptr_t) stype::rv_fsw), crypt::encrypt_ptr((uintptr_t) stype::rv_fsd),

		// BTYPE
		crypt::encrypt_ptr((uintptr_t) btype::rv_beq), crypt::encrypt_ptr((uintptr_t) btype::rv_bne),
		crypt::encrypt_ptr((uintptr_t) btype::rv_blt), crypt::encrypt_ptr((uintptr_t) btype::rv_bge),
		crypt::encrypt_ptr((uintptr_t) btype::rv_bltu), crypt::encrypt_ptr((uintptr_t) btype::rv_bgeu),

		// UTYPE/JTYPE
		crypt::encrypt_ptr((uintptr_t) utype::rv_lui), crypt::encrypt_ptr((uintptr_t) utype::rv_auipc),
		crypt::encrypt_ptr((uintptr_t) jtype::rv_jal)
	};
}
#endif // VMOPS_H
