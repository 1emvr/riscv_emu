#ifndef RV32_REGISTERS_HPP
#define RV32_REGISTERS_HPP
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

#pragma region REG_NATIVE_RESERVED
#define REG_FRAME  "rbp"
#define REG_STACK  "rsp"
#define REG_ARG0   "rcx"
#define REG_ARG1   "rdx"
#define REG_ARG2   "r8"
#define REG_ARG3   "r9"
#define REG_RETVAL "rax"
#pragma endregion // REG_NATIVE_RESERVED

#pragma region REG_RANDOM_ASSIGN
#define REG_VIP     "rsi"
#define REG_VSP     "rdi"
#define REG_VREG    "rbx"
#define REG_DKEY    "r10"
#define REG_SKR1    "r11"
#define REG_SKR2    "r12"
#define REG_OPND    "r13"
#define REG_HNDLR   "r14"
#define REG_PROG    "r15"
#pragma endregion // REG_RANDOM_ASSIGN

#pragma region REGENUM
#define REGENUM_ZR  0
#define REGENUM_RA  1
#define REGENUM_SP  2 
#define REGENUM_GP  3 
#define REGENUM_TP  4
#define REGENUM_T0  5
#define REGENUM_T1  6
#define REGENUM_T2  7
#define REGENUM_S0  8
#define REGENUM_S1  9
#define REGENUM_A0  10
#define REGENUM_A1  11
#define REGENUM_A2  12 
#define REGENUM_A3  13
#define REGENUM_A4  14
#define REGENUM_A5  15
#define REGENUM_A6  16
#define REGENUM_A7  17  
#define REGENUM_S2  18
#define REGENUM_S3  19
#define REGENUM_S4  20
#define REGENUM_S5  21
#define REGENUM_S6  22
#define REGENUM_S7  23
#define REGENUM_S8  24
#define REGENUM_S9  25
#define REGENUM_S10 26
#define REGENUM_S11 27  
#define REGENUM_T3  28
#define REGENUM_T4  29
#define REGENUM_T5  30
#define REGENUM_T6  31  
#pragma endregion // REGENUM

#pragma region VM_CAPACITY
#define VM_NATIVE_STACK_ALLOC             0x210
#define PROCESS_MAX_CAPACITY              (1024 * 256) 
#define VSTACK_MAX_CAPACITY               ((1024 * 16) + (16) * PTR_SIZE) // extra 2-uintptr for a guard page
#define VSCRATCH_MAX_CAPACITY             (64 * PTR_SIZE)
#define VREGS_MAX_CAPACITY                (32 * PTR_SIZE)

#define VREGS_SIZE                        (VREGS_MAX_CAPACITY * PTR_SIZE)
#define VSCRATCH_SIZE                     (VSCRATCH_MAX_CAPACITY * PTR_SIZE)
#pragma endregion // VM_CAPACITY

#pragma region VMCS_OFFSETS
#define VMCS_OFFSET_MOD_BASE              (PTR_SIZE * 0)
#define VMCS_OFFSET_DKEY                  (PTR_SIZE * 1)
#define VMCS_OFFSET_PROGRAM               (PTR_SIZE * 2)
#define VMCS_OFFSET_PROGRAM_SIZE          (PTR_SIZE * 3)
#define VMCS_OFFSET_VSTACK                (PTR_SIZE * 4)
#define VMCS_OFFSET_VSCRATCH              (PTR_SIZE * 4 + VSTACK_MAX_CAPACITY)
#define VMCS_OFFSET_VREGS                 (PTR_SIZE * 4 + VSTACK_MAX_CAPACITY + VSCRATCH_MAX_CAPACITY)
#pragma endregion // VMCS_OFFSETS

#define SP_OFFSET                         VMCS_OFFSET_VREGS + PTR_SIZE * REGENUM_SP
#define EXPONENT_MASK                     0x7FF0000000000000ULL
#define FRACTION_MASK                     0x000FFFFFFFFFFFFFULL

typedef enum {
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
} vm_reason;

typedef enum {
	zr, ra, sp, gp, tp, 
	t0, t1, t2, s0, s1,
	a0, a1, a2, a3, a4, a5, a6, a7,     
	s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,    
	t3, t4, t5, t6,     
} regenum;

typedef enum {
	rtype = 1, r4type, itype, stype, btype, utype, jtype, ftype,
} typenum;

typedef enum tblenum : uintptr_t {
	_addi, _slti, _sltiu, _xori, _ori, _andi, _slli, _srli,
	_srai, _addiw, _slliw, _srliw, _sraiw, _lb, _lh, _lw,
	_lbu, _lhu, _lwu, _ld, _flq, _fence, _fence_i, _jalr,
	_ecall, _ebreak, _csrrw, _csrrs, _csrrc, _csrrwi, _csrrsi, _csrrci,

	_fadd_d, _fsub_d, _fmul_d, _fdiv_d, _fsqrt_d, _fmv_d_x,
	_fcvt_s_d, _fcvt_s_q, _fcvt_d_s, _fcvt_d_q, _fcvt_w_d, _fcvt_wu_d,
	_fcvt_l_d, _fcvt_lu_d, _fcvt_d_w, _fcvt_d_wu, _fcvt_d_l, _fcvt_d_lu,

	_fsgnj_d, _fsgnjn_d, _fsgnjx_d, _fmin_d, _fmax_d, _feq_d,
	_flt_d, _fle_d, _fclass_d, _fmv_x_d,

	_lrw, _scw, _amoswap_w, _amoadd_w, _amoxor_w, _amoand_w,
	_amoor_w, _amomin_w, _amomax_w, _amominu_w, _amomaxu_w,

	_lrd, _scd, _amoswap_d, _amoadd_d, _amoxor_d, _amoand_d,
	_amoor_d, _amomin_d, _amomax_d, _amominu_d, _amomaxu_d,

	_addw, _subw, _mulw, _srlw, _sraw, _divuw, _sllw, _divw,
	_remw, _remuw, _add, _sub, _mul, _sll, _mulh, _slt,
	_mulhsu, _sltu, _mulhu, _xor, _div, _srl, _sra, _divu,
	_or, _rem, _and, _remu,

	_sb, _sh, _sw, _sd, _fsw, _fsd, _fsq,
	_beq, _bne, _blt, _bge, _bltu, _bgeu,
	_lui, _auipc, _jal
};

#define mem_read(T, addr, retval)				\
	if (addr % 4 != 0) {						\
		vmcs.halt = 1;							\
		vmcs.reason = unaligned_op;				\
		return;									\
	}											\
	if (addr >= PROCESS_MAX_CAPACITY || addr < vmcs.program) {		\
		vmcs.halt = 1;							\
		vmcs.reason = access_violation;			\
		return;									\
	}											\
												\
	size_t index = addr / sizeof(T);			\
	retval = vmcs.program[index]

#define mem_write(T, addr, value)				\
	if (addr % 4 != 0) {						\
		vmcs.halt = 1;							\
		vmcs.reason = unaligned_op;				\
		return;									\
	}											\
	if (addr >= PROCESS_MAX_CAPACITY) {			\
		vmcs.halt = 1;							\
		vmcs.reason = access_violation;			\
		return;									\
	}											\
												\
	size_t index = addr / sizeof(T);			\
	vmcs.program[index] = value;

#define reg_read(T, idx, retval)				\
	do {										\
		if ((idx) > REGENUM_T6) {				\
			vmcs.halt = 1;						\
			vmcs.reason = access_violation;		\
			return;								\
		}										\
		retval = (T)vmcs.vregs[(idx)];			\
	} while(0)

#define reg_write(T, idx, value)							\
	do {                                                    \
		if ((idx) == REGENUM_ZR || (idx) > REGENUM_T6) {	\
			vmcs.halt = 1;									\
			vmcs.reason = access_violation;					\
			return;                                         \
		}                                                   \
		vmcs.vregs[(idx)] = (T)value;						\
	} while (0)

#define ip_write(target)						\
	asm volatile ( "mov " REG_VIP ", %0" :: "r"(target) )

#define ip_read(retval)							\
	asm volatile ( "mov %0, " REG_VIP : "=r"(retval) )

#define set_branch(target)						\
	do {										\
		ip_write(target);						\
		vmcs.step = false;						\
	} while (0)

#endif // RV32_REGISTERS_HPP

