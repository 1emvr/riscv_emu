#include "vmmain.h"

namespace rvm64::decoder {
    struct opcode {
        uint8_t mask;
        typenum type;
    };
    // TODO: revise encoding and ratify which operations belong to what class/extension
    __rdata const opcode encoding[] = {
        { 0b1010011, rtype  }, { 0b1000011, rtype  }, { 0b0110011, rtype  }, { 0b1000111, r4type }, { 0b1001011, r4type }, { 0b1001111, r4type },
        { 0b0000011, itype  }, { 0b0001111, itype  }, { 0b1100111, itype  }, { 0b0010011, itype  }, { 0b1110011, itype  }, { 0b0100011, stype  },
        { 0b0100111, stype  }, { 0b1100011, btype  }, { 0b0010111, utype  }, { 0b0110111, utype  }, { 0b1101111, jtype  },
    };

    constexpr int32_t sign_extend(int32_t val, int bits) {
        int shift = 32 - bits;
        return (val << shift) >> shift;
    }

    inline int32_t imm_i(uint32_t opcode) {
        return (int32_t)opcode >> 20;
    }

    inline int32_t imm_s(uint32_t opcode) {
        return sign_extend(((opcode >> 25) << 5) | ((opcode >> 7) & 0x1F), 12);
    }

    inline int32_t imm_b(uint32_t opcode) {
        int32_t val = (((opcode >> 31) & 1) << 12)
                    | (((opcode >> 25) & 0x3F) << 5)
                    | (((opcode >> 8) & 0xF) << 1)
                    | (((opcode >> 7) & 1) << 11);
        return sign_extend(val, 13);
    }

    inline int32_t imm_u(uint32_t opcode) {
        return opcode & 0xFFFFF000;
    }

    inline int32_t imm_j(uint32_t opcode) {
        int32_t val = (((opcode >> 31) & 1) << 20)
                    | (((opcode >> 21) & 0x3FF) << 1)
                    | (((opcode >> 20) & 1) << 11)
                    | (((opcode >> 12) & 0xFF) << 12);
        return sign_extend(val, 21);
    }

    __function uintptr_t vm_decode(uint32_t opcode) {
        uint8_t decoded = 0;
        uint8_t opcode7 = opcode & 0x7F;

        for (int idx = 0; idx < sizeof(encoding); idx++) {
            if (encoding[idx].mask == opcode7) {
                decoded = encoding[idx].type;
                break;
            }
        }
        if (!decoded) {
            vmcs->halt = 1;
            vmcs->reason = illegal_op;
            return -1;
        }

        uint8_t func7 = (opcode >> 24) & 0x7F;
        uint8_t func5 = (func7 >> 2) & 0x1F;
        uint8_t func3 = (opcode >> 12) & 0x7;
        uint8_t func2 = (opcode >> 24) & 0x3;

        uint8_t rs3   = (opcode >> 27) & 0x1F;
        uint8_t rs2   = (opcode >> 20) & 0x1F; // also "shamt"
        uint8_t rs1   = (opcode >> 15) & 0x1F;
        uint8_t rd    = (opcode >> 7) & 0x1F;

        int32_t _imm_i = imm_i(opcode);
        int32_t _imm_s = imm_s(opcode);
        int32_t _imm_u = imm_u(opcode);
        int32_t _imm_b = imm_b(opcode);
        int32_t _imm_j = imm_j(opcode);

        vmcs->vscratch1[0] = func3;
        vmcs->vscratch1[1] = rd;
        vmcs->vscratch1[2] = rs1;
        vmcs->vscratch1[3] = rs2;
        vmcs->vscratch1[4] = rs3;
        vmcs->vscratch1[5] = func7;
        vmcs->vscratch1[6] = func5;
        vmcs->vscratch1[7] = func2;

        vmcs->vscratch2[0] = _imm_i;
        vmcs->vscratch2[1] = _imm_s;
        vmcs->vscratch2[2] = _imm_b;
        vmcs->vscratch2[3] = _imm_u;
        vmcs->vscratch2[4] = _imm_j;

        switch(decoded) {
            // I_TYPE
            case itype: switch(opcode7) {
                case 0b0010011: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_addi];
                    case 0b010: return ((uintptr_t*)vmcs->handler)[_rv_slti];
                    case 0b011: return ((uintptr_t*)vmcs->handler)[_rv_sltiu];
                    case 0b100: return ((uintptr_t*)vmcs->handler)[_rv_xori];
                    case 0b110: return ((uintptr_t*)vmcs->handler)[_rv_ori];
                    case 0b111: return ((uintptr_t*)vmcs->handler)[_rv_andi];
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_slli];
                    case 0b101: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_srli];
                        case 0b0100000: return ((uintptr_t*)vmcs->handler)[_rv_srai];
                    }
                }
                case 0b0011011: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_addiw];
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_slliw];
                    case 0b101: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_srliw];
                        case 0b0100000: return ((uintptr_t*)vmcs->handler)[_rv_sraiw];
                    }
                }
                case 0b0000011: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_lb];
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_lh];
                    case 0b010: return ((uintptr_t*)vmcs->handler)[_rv_lw];
                    case 0b100: return ((uintptr_t*)vmcs->handler)[_rv_lbu];
                    case 0b101: return ((uintptr_t*)vmcs->handler)[_rv_lhu];
                    case 0b110: return ((uintptr_t*)vmcs->handler)[_rv_lwu];
                    case 0b011: return ((uintptr_t*)vmcs->handler)[_rv_ld];
                }
            }

                // R_TYPE
            case rtype: switch(opcode7) {
                case 0b1010011: switch(func7) {
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_fadd_d];
                    case 0b0000101: return ((uintptr_t*)vmcs->handler)[_rv_fsub_d];
                    case 0b0001001: return ((uintptr_t*)vmcs->handler)[_rv_fmul_d];
                    case 0b0001101: return ((uintptr_t*)vmcs->handler)[_rv_fdiv_d];
                    case 0b0101101: return ((uintptr_t*)vmcs->handler)[_rv_fsqrt_d];
                    case 0b1111001: return ((uintptr_t*)vmcs->handler)[_rv_fmv_d_x];
                    case 0b0100000: switch(rs2) {
                        case 0b00001: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_s_d];
                        case 0b00011: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_s_q];
                    }
                    case 0b0100001: switch(rs2) {
                        case 0b00000: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_d_s];
                        case 0b00011: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_d_q];
                    }
                    case 0b1100001: switch(rs2) {
                        case 0b00000: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_w_d];
                        case 0b00001: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_wu_d];
                        case 0b00010: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_l_d];
                        case 0b00011: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_lu_d];
                    }
                    case 0b1101001: switch(rs2) {
                        case 0b00000: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_d_w];
                        case 0b00001: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_d_wu];
                        case 0b00010: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_d_l];
                        case 0b00011: return ((uintptr_t*)vmcs->handler)[_rv_fcvt_d_lu];
                    }
                    case 0b0010001: switch(func3) {
                        case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_fsgnj_d];
                        case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_fsgnjn_d];
                        case 0b010: return ((uintptr_t*)vmcs->handler)[_rv_fsgnjx_d];
                    }
                    case 0b0010101: switch(func3) {
                        case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_fmin_d];
                        case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_fmax_d];
                    }
                    case 0b1010001: switch(func3) {
                        case 0b010: return ((uintptr_t*)vmcs->handler)[_rv_feq_d];
                        case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_flt_d];
                        case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_fle_d];
                    }
                    case 0b1110001: switch(func3) {
                        case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_fclass_d];
                        case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_fmv_x_d];
                    }
                }
                case 0b0101111: switch(func3) {
                    case 0b010: switch(func5) {
                        case 0b00010: return ((uintptr_t*)vmcs->handler)[_rv_lrw];
                        case 0b00011: return ((uintptr_t*)vmcs->handler)[_rv_scw];
                        case 0b00001: return ((uintptr_t*)vmcs->handler)[_rv_amoswap_w];
                        case 0b00000: return ((uintptr_t*)vmcs->handler)[_rv_amoadd_w];
                        case 0b00100: return ((uintptr_t*)vmcs->handler)[_rv_amoxor_w];
                        case 0b01100: return ((uintptr_t*)vmcs->handler)[_rv_amoand_w];
                        case 0b01000: return ((uintptr_t*)vmcs->handler)[_rv_amoor_w];
                        case 0b10000: return ((uintptr_t*)vmcs->handler)[_rv_amomin_w];
                        case 0b10100: return ((uintptr_t*)vmcs->handler)[_rv_amomax_w];
                        case 0b11000: return ((uintptr_t*)vmcs->handler)[_rv_amominu_w];
                        case 0b11100: return ((uintptr_t*)vmcs->handler)[_rv_amomaxu_w];
                    }
                    case 0b011: switch(func5) {
                        case 0b00010: return ((uintptr_t*)vmcs->handler)[_rv_lrd];
                        case 0b00011: return ((uintptr_t*)vmcs->handler)[_rv_scd];
                        case 0b00001: return ((uintptr_t*)vmcs->handler)[_rv_amoswap_d];
                        case 0b00000: return ((uintptr_t*)vmcs->handler)[_rv_amoadd_d];
                        case 0b00100: return ((uintptr_t*)vmcs->handler)[_rv_amoxor_d];
                        case 0b01100: return ((uintptr_t*)vmcs->handler)[_rv_amoand_d];
                        case 0b01000: return ((uintptr_t*)vmcs->handler)[_rv_amoor_d];
                        case 0b10000: return ((uintptr_t*)vmcs->handler)[_rv_amomin_d];
                        case 0b10100: return ((uintptr_t*)vmcs->handler)[_rv_amomax_d];
                        case 0b11000: return ((uintptr_t*)vmcs->handler)[_rv_amominu_d];
                        case 0b11100: return ((uintptr_t*)vmcs->handler)[_rv_amomaxu_d];
                    }
                }
                case 0b0111011: switch(func3) {
                    case 0b000: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_addw];
                        case 0b0100000: return ((uintptr_t*)vmcs->handler)[_rv_subw];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_mulw];
                    }
                    case 0b101: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_srlw];
                        case 0b0100000: return ((uintptr_t*)vmcs->handler)[_rv_sraw];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_divuw];
                    }
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_sllw];
                    case 0b100: return ((uintptr_t*)vmcs->handler)[_rv_divw];
                    case 0b110: return ((uintptr_t*)vmcs->handler)[_rv_remw];
                    case 0b111: return ((uintptr_t*)vmcs->handler)[_rv_remuw];
                }
                case 0b0110011: switch(func3) {
                    case 0b000: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_add];
                        case 0b0100000: return ((uintptr_t*)vmcs->handler)[_rv_sub];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_mul];
                    }
                    case 0b001: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_sll];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_mulh];
                    }
                    case 0b010: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_slt];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_mulhsu];
                    }
                    case 0b011: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_sltu];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_mulhu];
                    }
                    case 0b100: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_xor];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_div];
                    }
                    case 0b101: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_srl];
                        case 0b0100000: return ((uintptr_t*)vmcs->handler)[_rv_sra];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_divu];
                    }
                    case 0b110: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_or];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_rem];
                    }
                    case 0b111: switch(func7) {
                        case 0b0000000: return ((uintptr_t*)vmcs->handler)[_rv_and];
                        case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rv_remu];
                    }
                }
            }
                // S TYPE
            case stype: switch(opcode7) {
                case 0b0100011: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_sb];
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_sh];
                    case 0b010: return ((uintptr_t*)vmcs->handler)[_rv_sw];
                    case 0b011: return ((uintptr_t*)vmcs->handler)[_rv_sd];
                }
                case 0b0100111: switch(func3) {
                    case 0b010: return ((uintptr_t*)vmcs->handler)[_rv_fsw];
                    case 0b011: return ((uintptr_t*)vmcs->handler)[_rv_fsd];
                    case 0b100: return ((uintptr_t*)vmcs->handler)[_rv_fsq]; // TODO: whoops, deleted on accident
                }
            }
                // B TYPE
            case btype: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs->handler)[_rv_beq];
                case 0b001: return ((uintptr_t*)vmcs->handler)[_rv_bne];
                case 0b100: return ((uintptr_t*)vmcs->handler)[_rv_blt];
                case 0b101: return ((uintptr_t*)vmcs->handler)[_rv_bge];
                case 0b110: return ((uintptr_t*)vmcs->handler)[_rv_bltu];
                case 0b111: return ((uintptr_t*)vmcs->handler)[_rv_bgeu];
            }
                // U TYPE
            case utype: switch(opcode7) {
                case 0b0110111: return ((uintptr_t*)vmcs->handler)[_rv_lui];
                case 0b0010111: return ((uintptr_t*)vmcs->handler)[_rv_auipc];
            }
                // J TYPE
            case jtype: switch(opcode7) {
                case 0b1101111: return ((uintptr_t*)vmcs->handler)[_rv_jal];
            }
                // R4 TYPE
            case r4type: {}
        }
    }
};
