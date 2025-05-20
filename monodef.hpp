#ifndef RV32_REGISTERS_HPP
#define RV32_REGISTERS_HPP
#include <windows.h>
#include <stdint.h>

#define __extern   extern "C"
#define __vmcall   __attribute__((annotate("vm_calling_convention")))
#define __function //__attribute__((section(".text$B")))
#define __rdata    __attribute__((section(".rdata")))
#define __data     __attribute__((section(".data")))
#define __used     __attribute__((used))

#define NT_SUCCESS(Status)  ((Status) >= 0)
#define NtCurrentProcess()  ((HANDLE) (LONG_PTR) -1)
#define NtCurrentThread()   ((HANDLE) (LONG_PTR) -2)

#ifdef __x86_64__
#define PTR_SIZE 8                          
#else
#define PTR_SIZE 4                          
#endif

#pragma region VM_CAPACITY
#define VM_NATIVE_STACK_ALLOC             0x210
#define PROCESS_MAX_CAPACITY              (1024 * 256) 
#define VSTACK_MAX_CAPACITY               (1024 * 1024)
#pragma endregion // VM_CAPACITY

#define EXPONENT_MASK                     0x7FF0000000000000ULL
#define FRACTION_MASK                     0x000FFFFFFFFFFFFFULL

enum vm_reason {
	ok,
	illegal_op,
	unaligned_op,
	access_violation,
	return_address_corruption,
	stack_overflow,
	out_of_memory,
	undefined,
	user_ecall,
	visor_ecall,
	machine_ecall,
	debug_break,
};

enum typenum {
	rtype = 1, r4type, itype, stype, btype, utype, jtype, ftype,
};

enum HandlerIndex : uint8_t {
	// ITYPE
	_rv_addi, _rv_slti, _rv_sltiu, _rv_xori, _rv_ori, _rv_andi,
	_rv_slli, _rv_srli, _rv_srai, _rv_addiw, _rv_slliw, _rv_srliw,
	_rv_sraiw, _rv_lb, _rv_lh, _rv_lw, _rv_lbu, _rv_lhu, _rv_lwu, _rv_ld,
	_rv_flq, _rv_fence, _rv_fence_i, _rv_jalr, _rv_ecall, _rv_ebreak,
	_rv_csrrw, _rv_csrrs, _rv_csrrc, _rv_csrrwi, _rv_csrrsi, _rv_csrrci,
	_rv_fclass_d, _rv_lrw, _rv_lrd,

	// RTYPE
	_rv_fadd_d, _rv_fsub_d, _rv_fmul_d, _rv_fdiv_d, _rv_fsqrt_d, _rv_fmv_d_x,
	_rv_fcvt_s_d, _rv_fcvt_s_q, _rv_fcvt_d_s, _rv_fcvt_d_q,
	_rv_fcvt_w_d, _rv_fcvt_wu_d, _rv_fcvt_l_d, _rv_fcvt_lu_d,
	_rv_fcvt_d_w, _rv_fcvt_d_wu, _rv_fcvt_d_l, _rv_fcvt_d_lu,

	_rv_fsgnj_d, _rv_fsgnjn_d, _rv_fsgnjx_d, _rv_fmin_d, _rv_fmax_d,
	_rv_feq_d, _rv_flt_d, _rv_fle_d, _rv_fmv_x_d, _rv_scw,
	_rv_amoswap_w, _rv_amoadd_w, _rv_amoxor_w, _rv_amoand_w,
	_rv_amoor_w, _rv_amomin_w, _rv_amomax_w, _rv_amominu_w,
	_rv_amomaxu_w,

	_rv_scd, _rv_amoswap_d, _rv_amoadd_d, _rv_amoxor_d, _rv_amoand_d,
	_rv_amoor_d, _rv_amomin_d, _rv_amomax_d, _rv_amominu_d,
	_rv_amomaxu_d,

	_rv_addw, _rv_subw, _rv_mulw, _rv_srlw, _rv_sraw, _rv_divuw,
	_rv_sllw, _rv_divw, _rv_remw, _rv_remuw,

	_rv_add, _rv_sub, _rv_mul, _rv_sll, _rv_mulh, _rv_slt,
	_rv_mulhsu, _rv_sltu, _rv_mulhu, _rv_xor,
	_rv_div, _rv_srl, _rv_sra, _rv_divu, _rv_or, _rv_rem,
	_rv_and, _rv_remu,

	// STYPE
	_rv_sb, _rv_sh, _rv_sw, _rv_sd, _rv_fsw, _rv_fsd,

	// BTYPE
	_rv_beq, _rv_bne, _rv_blt, _rv_bge, _rv_bltu, _rv_bgeu,

	// UTYPE/JTYPE
	_rv_lui, _rv_auipc, _rv_jal,
};

enum skrenum {
    rd, rs1, rs2, rs3, imm,
};
enum regenum {
	zr, ra, sp, gp, tp,
	t0, t1, t2, s0, s1,
	a0, a1, a2, a3, a4, a5, a6, a7,
	s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
	t3, t4, t5, t6,
};

// NOTE: Probably not fool-proof. Could probably do something cynical.
// mem r/w don't follow the same convention as the other macros for some reason.
#define mem_read(T, retval, addr)                                               \
do {						                                                    \
    if ((addr) % sizeof(T) != 0) {												\
        vmcs->halt = 1;															\
        vmcs->reason = unaligned_op;											\
        return;																	\
    }																			\
    if ((addr) < (uintptr_t)vmcs->process.address || (addr) + sizeof(T) > PROCESS_MAX_CAPACITY) {	\
        vmcs->halt = 1;															\
        vmcs->reason = access_violation;										\
        return;																	\
    }																			\
    retval = *(T*) (vmcs->process.address + ((addr) - vmcs->process.address));	\
} while (0)

#define mem_write(T, addr, value)                                               \
do {											                                \
    if ((addr) % sizeof(T) != 0) {												\
        vmcs->halt = 1;															\
        vmcs->reason = unaligned_op;											\
        return;																	\
    }																			\
    if ((addr) < vmcs->process.address || (addr) + sizeof(T) > PROCESS_MAX_CAPACITY) {	\
        vmcs->halt = 1;															\
        vmcs->reason = access_violation;										\
        return;																	\
    }																			\
    *(T*) (vmcs->process.address + ((addr) - vmcs->process.address)) = (value); \
} while (0)

#define reg_read(T, dst, src)					\
	do {										\
		if ((src) > regenum::t6) {				\
			vmcs->halt = 1;						\
			vmcs->reason = access_violation;	\
			return;								\
		}										\
		dst = (T)vmcs->vregs[(src)];			\
	} while(0)

#define reg_write(T, dst, src)								\
	do {                                                    \
		if ((dst) == regenum::zr || (dst) > regenum::t6) {	\
			vmcs->halt = 1;									\
			vmcs->reason = access_violation;				\
			return;                                         \
		}                                                   \
		vmcs->vregs[(dst)] = (T)src;						\
	} while (0)

#define scr_read(T, dst, src)								\
	do {													\
		if (src > skrenum::imm) {							\
			vmcs->halt = 1;									\
			vmcs->reason = access_violation;				\
			return;                                         \
		}													\
		dst = (T)vmcs->vscratch[(src)];						\
	} while (0)

#define scr_write(T, dst, src)								\
	do {													\
		if (dst > skrenum::imm) {							\
			vmcs->halt = 1;									\
			vmcs->reason = access_violation;				\
			return;                                         \
		}													\
		vmcs->vscratch[(dst)] = (T)src;						\
	} while (0)

#define unwrap_opcall(idx) ((void (*)(void))((uintptr_t*)vmcs->handler)[idx])();

#define set_branch(target)						\
	do {										\
		ip_write(target);						\
		vmcs->step = false;						\
	} while (0)

#endif // RV32_REGISTERS_HPP

