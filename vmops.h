#ifndef VMOPS_H
#define VMOPS_H
#include "mono.hpp"
#include "vmcrypt.h"

namespace rvm64::operation {
    bool is_nan(double x);

    // i-type
    namespace itype {
        __function void rv_lrw();
        __function void rv_lrd();
        __function void rv_fmv_d_x();

        __function void rv_fcvt_s_d();
        __function void rv_fcvt_d_s();
        __function void rv_fcvt_w_d();
        __function void rv_fcvt_wu_d();
        __function void rv_fcvt_d_w();
        __function void rv_fcvt_d_wu();
        __function void rv_fclass_d();

        __function void rv_addi();
        __function void rv_slti();
        __function void rv_sltiu();
        __function void rv_xori();
        __function void rv_ori();
        __function void rv_andi();
        __function void rv_slli();
        __function void rv_srli();
        __function void rv_srai();
        __function void rv_addiw();
        __function void rv_slliw();
        __function void rv_srliw();
        __function void rv_sraiw();

        __function void rv_lb();
        __function void rv_lh();
        __function void rv_lw();
        __function void rv_lbu();
        __function void rv_lhu();
        __function void rv_lwu();
        __function void rv_ld();
        __function void rv_flq();

        __function void rv_fence();
        __function void rv_fence_i();
        __function void rv_jalr();
        __function void rv_ecall();
        __function void rv_ebreak();
        __function void rv_csrrw();
        __function void rv_csrrs();
        __function void rv_csrrc();
        __function void rv_csrrwi();
        __function void rv_csrrsi();
        __function void rv_csrrci();
    };

    // r-type
    namespace rtype {
        __function void rv_scw();
        __function void rv_scd();
        __function void rv_fadd_d();
        __function void rv_fsub_d();
        __function void rv_fmul_d();
        __function void rv_fdiv_d();
        __function void rv_fsgnj_d();
        __function void rv_fsgnjn_d();
        __function void rv_fsgnjx_d();
        __function void rv_fmin_d();
        __function void rv_fmax_d();
        __function void rv_feq_d();
        __function void rv_flt_d();
        __function void rv_fle_d();
        __function void rv_addw();
        __function void rv_subw();
        __function void rv_mulw();
        __function void rv_srlw();
        __function void rv_sraw();
        __function void rv_divuw();
        __function void rv_sllw();
        __function void rv_divw();
        __function void rv_remw();
        __function void rv_remuw();
        __function void rv_add();
        __function void rv_sub();
        __function void rv_mul();
        __function void rv_sll();
        __function void rv_mulh();
        __function void rv_slt();
        __function void rv_mulhsu();
        __function void rv_sltu();
        __function void rv_mulhu();
        __function void rv_xor();
        __function void rv_div();
        __function void rv_srl();
        __function void rv_sra();
        __function void rv_divu();
        __function void rv_or();
        __function void rv_rem();
        __function void rv_and();
        __function void rv_remu();
        __function void rv_fsqrt_d(); // NOTE: I'M VERY SURE THIS IS ITYPE, NOT RTYPE....
        __function void rv_fmv_d_x();
        __function void rv_fcvt_s_d();
        __function void rv_fcvt_s_q();
        __function void rv_fcvt_d_s();
        __function void rv_fcvt_d_q();
        __function void rv_fcvt_w_d();
        __function void rv_fcvt_wu_d();
        __function void rv_fcvt_l_d();
        __function void rv_fcvt_lu_d();
        __function void rv_fcvt_d_w();
        __function void rv_fcvt_d_wu();
        __function void rv_fcvt_d_l();
        __function void rv_fcvt_d_lu();
        __function void rv_amoswap_d();
        __function void rv_amoadd_d();
        __function void rv_amoxor_d();
        __function void rv_amoand_d();
        __function void rv_amoor_d();
        __function void rv_amomin_d();
        __function void rv_amomax_d();
        __function void rv_amominu_d();
        __function void rv_amomaxu_d();
        __function void rv_amoswap_w();
        __function void rv_amoadd_w();
        __function void rv_amoxor_w();
        __function void rv_amoand_w();
        __function void rv_amoor_w();
        __function void rv_amomin_w();
        __function void rv_amomax_w();
        __function void rv_amominu_w();
        __function void rv_amomaxu_w();
    };

    namespace stype {
        __function void rv_sb();
        __function void rv_sh();
        __function void rv_sw();
        __function void rv_sd();
        __function void rv_fsw();
        __function void rv_fsd();
    };

    namespace btype {
        __function void rv_beq();
        __function void rv_bne();
        __function void rv_blt();
        __function void rv_bge();
        __function void rv_bltu();
        __function void rv_bgeu();
    };

    namespace utype {
        __function void rv_lui();
        __function void rv_auipc();
    };

    namespace jtype {
        __function void rv_jal();
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
        crypt::encrypt_ptr((uintptr_t) itype::rv_fclass_d), crypt::encrypt_ptr((uintptr_t) itype::rv_lrw),
        crypt::encrypt_ptr((uintptr_t) itype::rv_lrd),

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
        crypt::encrypt_ptr((uintptr_t) rtype::rv_fmv_x_d), crypt::encrypt_ptr((uintptr_t) rtype::rv_scw),
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
};
#endif //VMOPS_H
