#ifndef VMOPS_H
#define VMOPS_H
#include "mono.hpp"
#include "vmcrypt.h"

namespace rvm64::operation {
    bool is_nan(double x);

    // i-type
    namespace itype {
        __function void rv_lrw(uint8_t rs1, uint8_t rd);
        __function void rv_lrd(uint8_t rs1, uint8_t rd);
        __function void rv_fmv_d_x(uint8_t rs1, uint8_t rd);

        __function void rv_fcvt_s_d(uint8_t rs1, uint8_t rd);
        __function void rv_fcvt_d_s(uint8_t rs1, uint8_t rd);
        __function void rv_fcvt_w_d(uint8_t rs1, uint8_t rd);
        __function void rv_fcvt_wu_d(uint8_t rs1, uint8_t rd);
        __function void rv_fcvt_d_w(uint8_t rs1, uint8_t rd);
        __function void rv_fcvt_d_wu(uint8_t rs1, uint8_t rd);
        __function void rv_fclass_d(uint8_t rs1, uint8_t rd);

        __function void rv_addi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_slti(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_sltiu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_xori(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_ori(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_andi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_slli(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_srli(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_srai(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_addiw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_slliw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_srliw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_sraiw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);

        __function void rv_lb(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_lh(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_lw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_lbu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_lhu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_lwu(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_ld(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_flq(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);

        __function void rv_fence(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_fence_i(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_jalr(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_ecall(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_ebreak(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_csrrw(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_csrrs(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_csrrc(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_csrrwi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_csrrsi(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
        __function void rv_csrrci(uint16_t imm_11_0, uint8_t rs1, uint8_t rd);
    };

    // r-type
    namespace rtype {
        __function void rv_scw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_scd(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fadd_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fsub_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fmul_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fdiv_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fsgnj_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fsgnjn_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fsgnjx_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fmin_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fmax_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_feq_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_flt_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_fle_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_addw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_subw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_mulw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_srlw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_sraw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_divuw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_sllw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_divw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_remw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_remuw(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_add(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_sub(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_mul(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_sll(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_mulh(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_slt(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_mulhsu(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_sltu(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_mulhu(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_xor(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_div(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_srl(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_sra(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_divu(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_or(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_rem(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_and(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_remu(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoswap_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoadd_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoxor_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoand_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoor_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amomin_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amomax_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amominu_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amomaxu_d(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoswap_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoadd_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoxor_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoand_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amoor_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amomin_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amomax_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amominu_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
        __function void rv_amomaxu_w(uint8_t rs2, uint8_t rs1, uint8_t rd);
    };

    namespace stype {
        __function void rv_sb(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0);
        __function void rv_sh(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0);
        __function void rv_sw(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0);
        __function void rv_sd(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0);
        __function void rv_fsw(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0);
        __function void rv_fsd(uint8_t imm_11_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_0);
    };

    namespace btype {
        __function void rv_beq(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11);
        __function void rv_bne(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11);
        __function void rv_blt(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11);
        __function void rv_bge(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11);
        __function void rv_bltu(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11);
        __function void rv_bgeu(uint8_t imm_12_10_5, uint8_t rs2, uint8_t rs1, uint8_t imm_4_1_11);
    };

    namespace utype {
        __function void rv_lui(uint32_t imm_31_12, uint8_t rd);
        __function void rv_auipc(uint32_t imm_31_12, uint8_t rd);
    };

    namespace jtype {
        __function void rv_jal(uint32_t imm_20_10_1_11_19_12, uint8_t rd);
    };

    __rdata constexpr uintptr_t __handler[256] = {
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

        // RTYPE
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fadd_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fsub_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fmul_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fdiv_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fsqrt_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fmv_d_x),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_s_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_s_q),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_d_s), crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_d_q),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_w_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_wu_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_l_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_lu_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_d_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_d_wu),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_d_l), crypt::encrypt_ptr((uintptr_t) rtype::rv_fcvt_d_lu),

        crypt::encrypt_ptr((uintptr_t) rtype::rv_fsgnj_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fsgnjn_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fsgnjx_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fmin_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fmax_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_feq_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_flt_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fle_d),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fclass_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_fmv_x_d),

        crypt::encrypt_ptr((uintptr_t) rtype::rv_lrw), crypt::encrypt_ptr((uintptr_t) rtype::rv_scw),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_amoswap_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amoadd_w),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_amoxor_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amoand_w),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_amoor_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amomin_w),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_amomax_w), crypt::encrypt_ptr((uintptr_t) rtype::rv_amominu_w),
        crypt::encrypt_ptr((uintptr_t) rtype::rv_amomaxu_w),

        crypt::encrypt_ptr((uintptr_t) rtype::rv_lrd), crypt::encrypt_ptr((uintptr_t) rtype::rv_scd),
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

        // STYPE
        crypt::encrypt_ptr((uintptr_t) stype::rv_sb), crypt::encrypt_ptr((uintptr_t) stype::rv_sh),
        crypt::encrypt_ptr((uintptr_t) stype::rv_sw), crypt::encrypt_ptr((uintptr_t) stype::rv_sd),
        crypt::encrypt_ptr((uintptr_t) stype::rv_fsw), crypt::encrypt_ptr((uintptr_t) stype::rv_fsd),
        crypt::encrypt_ptr((uintptr_t) stype::rv_fsq),

        // BTYPE
        crypt::encrypt_ptr((uintptr_t) btype::rv_beq), crypt::encrypt_ptr((uintptr_t) btype::rv_bne),
        crypt::encrypt_ptr((uintptr_t) btype::rv_blt), crypt::encrypt_ptr((uintptr_t) btype::rv_bge),
        crypt::encrypt_ptr((uintptr_t) btype::rv_bltu), crypt::encrypt_ptr((uintptr_t) btype::rv_bgeu),

        // UTYPE/JTYPE
        crypt::encrypt_ptr((uintptr_t) utype::rv_lui), crypt::encrypt_ptr((uintptr_t) utype::rv_auipc),
        crypt::encrypt_ptr((uintptr_t) jtype::rv_jal)
    };
};
#endif //VMOPS_H
