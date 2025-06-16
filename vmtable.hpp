#ifndef VMTABLE_HPP
#define VMTABLE_HPP
#include "vmops.hpp"

__rdata const uintptr_t __handler[256] = {
	// ITYPE
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_addi), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_slti),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_sltiu), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_xori),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ori), 			rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_andi),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_slli), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_srli),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_srai), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_addiw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_slliw), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_srliw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_sraiw), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lb),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lh), 			rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lw),

	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lbu), 			rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lhu),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lwu), 			rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ld),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_flq), 			rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fence),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fence_i), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_jalr),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ecall), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_ebreak),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrw), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrs),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrc), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrwi),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrsi), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_csrrci),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fclass_d), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lrw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_lrd), 			rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fmv_d_x),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_s_d), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_d_s),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_w_d), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_wu_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_d_w), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::itype::rv_fcvt_d_wu),

	// RTYPE
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fadd_d), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsub_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fmul_d), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fdiv_d),
	/*rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsqrt_d),*/ 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsgnj_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsgnjn_d), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fsgnjx_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fmin_d), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fmax_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_feq_d), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_flt_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_fle_d), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_scw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoswap_w), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoadd_w),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoxor_w), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoand_w),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoor_w), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomin_w),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomax_w), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amominu_w),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomaxu_w),

	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_scd),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoswap_d), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoadd_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoxor_d), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoand_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amoor_d), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomin_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomax_d), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amominu_d),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_amomaxu_d),

	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_addw), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_subw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulw), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_srlw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sraw), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_divuw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sllw), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_divw),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_remw), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_remuw),

	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_add), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sub),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mul), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sll),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulh), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_slt),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulhsu), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sltu),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_mulhu), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_xor),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_div), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_srl),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_sra), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_divu),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_or), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_rem),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_and), 		rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::rtype::rv_remu),

	// TODO: Finish floating point instructions (probably S and I types) - f, fm, fnm, fcvt - have a few already done.
	// STYPE
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sb), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sh),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sw), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_sd),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_fsw), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::stype::rv_fsd),

	// BTYPE
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_beq), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bne),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_blt), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bge),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bltu), rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::btype::rv_bgeu),

	// UTYPE/JTYPE
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::utype::rv_lui), 	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::utype::rv_auipc),
	rvm64::crypt::encrypt_ptr((uintptr_t) rvm64::operations::jtype::rv_jal)
};
#endif VMTABLE_HPP
