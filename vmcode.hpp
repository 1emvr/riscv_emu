#ifndef VMCODE_H
#define VMCODE_H
#include "vmmain.hpp"

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

    inline uint8_t shamt_i(uint32_t opcode) {
        return (opcode >> 20) & 0x1F;
    }

    // NOTE: annoying as fuck to read. just let GPT do the math and say fuck it.
    inline int32_t imm_u(uint32_t opcode) { return opcode & 0xFFFFF000; }
    inline int32_t imm_i(uint32_t opcode) { return (int32_t)opcode >> 20; }
    inline int32_t imm_s(uint32_t opcode) { return sign_extend(((opcode >> 25) << 5) | ((opcode >> 7) & 0x1F), 12); }
    inline int32_t imm_b(uint32_t opcode) {
        int32_t val = (((opcode >> 31) & 1) << 12)
                    | (((opcode >> 25) & 0x3F) << 5)
                    | (((opcode >> 8) & 0xF) << 1)
                    | (((opcode >> 7) & 1) << 11);
        return sign_extend(val, 13);
    }

    inline int32_t imm_j(uint32_t opcode) {
        int32_t val = (((opcode >> 31) & 1) << 20)
                    | (((opcode >> 21) & 0x3FF) << 1)
                    | (((opcode >> 20) & 1) << 11)
                    | (((opcode >> 12) & 0xFF) << 12);
        return sign_extend(val, 21);
    }

    // TODO: pointers in handler are never decrypted before calling unwrap_opcall()

    __function void vm_decode(uint32_t opcode) {
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
            return;
        }

        uint8_t func7 = (opcode >> 24) & 0x7F;
        uint8_t func5 = (func7 >> 2) & 0x1F;
        uint8_t func3 = (opcode >> 12) & 0x7;
        uint8_t func2 = (opcode >> 24) & 0x3;

        uint8_t rs3   = (opcode >> 27) & 0x1F;
        uint8_t rs2   = (opcode >> 20) & 0x1F; // also "shamt"
        uint8_t rs1   = (opcode >> 15) & 0x1F;
        uint8_t rd    = (opcode >> 7) & 0x1F;

        uint32_t _shamt = shamt_i(opcode);
        int32_t _imm_i = imm_i(opcode);
        int32_t _imm_s = imm_s(opcode);
        int32_t _imm_u = imm_u(opcode);
        int32_t _imm_b = imm_b(opcode);
        int32_t _imm_j = imm_j(opcode);

		// pass bitfields into scratch registers
        vmcs->vscratch[0] = rd;
        vmcs->vscratch[2] = rs1;
        vmcs->vscratch[3] = rs2;
        vmcs->vscratch[4] = rs3;

	// TODO: call - do not return
        switch(decoded) {
            // I_TYPE
            case itype:
                scr_write(int32_t, imm, _imm_i);
                switch(opcode7) {
                case 0b0010011: switch(func3) {
                    case 0b000: unwrap_opcall(_rv_addi); break;
                    case 0b010: unwrap_opcall(_rv_slti); break;
                    case 0b011: unwrap_opcall(_rv_sltiu); break;
                    case 0b100: unwrap_opcall(_rv_xori); break;
                    case 0b110: unwrap_opcall(_rv_ori); break;
                    case 0b111: unwrap_opcall(_rv_andi); break;

                        // separate scenario where the function takes the imm as shamt
                    case 0b001: scr_write(int32_t, imm, _shamt); unwrap_opcall(_rv_slli); break;
                    case 0b101: switch(func7) {
                        case 0b0000000: scr_write(int32_t, imm, _shamt); unwrap_opcall(_rv_srli); break;
                        case 0b0100000: scr_write(int32_t, imm, _shamt); unwrap_opcall(_rv_srai); break;
                    }
                }
                case 0b0011011: switch(func3) {
                    case 0b000: unwrap_opcall(_rv_addiw); break;

                        // separate scenario where the function takes the imm as shamt
                    case 0b001: scr_write(int32_t, imm, _shamt); unwrap_opcall(_rv_slliw); break;
                    case 0b101: switch(func7) {
                        case 0b0000000: scr_write(int32_t, imm, _shamt); unwrap_opcall(_rv_srliw); break;
                        case 0b0100000: scr_write(int32_t, imm, _shamt); unwrap_opcall(_rv_sraiw); break;
                    }
                }
                case 0b0000011: switch(func3) {
                    case 0b000: unwrap_opcall(_rv_lb); break;
                    case 0b001: unwrap_opcall(_rv_lh); break;
                    case 0b010: unwrap_opcall(_rv_lw); break;
                    case 0b100: unwrap_opcall(_rv_lbu); break;
                    case 0b101: unwrap_opcall(_rv_lhu); break;
                    case 0b110: unwrap_opcall(_rv_lwu); break;
                    case 0b011: unwrap_opcall(_rv_ld); break;
                }
			default:
				vmcs->halt = 1;
				vmcs->reason = illegal_op;
				return;
            }

                // R_TYPE
            case rtype: switch(opcode7) {
                case 0b1010011: switch(func7) {
                    case 0b0000001: unwrap_opcall(_rv_fadd_d); break;
                    case 0b0000101: unwrap_opcall(_rv_fsub_d); break;
                    case 0b0001001: unwrap_opcall(_rv_fmul_d); break;
                    case 0b0001101: unwrap_opcall(_rv_fdiv_d); break;
                    case 0b0101101: unwrap_opcall(_rv_fsqrt_d); break;
                    case 0b1111001: unwrap_opcall(_rv_fmv_d_x); break;
                    case 0b0100000: switch(rs2) {
                        case 0b00001: unwrap_opcall(_rv_fcvt_s_d); break;
                        case 0b00011: unwrap_opcall(_rv_fcvt_s_q); break;
                    }
                    case 0b0100001: switch(rs2) {
                        case 0b00000: unwrap_opcall(_rv_fcvt_d_s); break;
                        case 0b00011: unwrap_opcall(_rv_fcvt_d_q); break;
                    }
                    case 0b1100001: switch(rs2) {
                        case 0b00000: unwrap_opcall(_rv_fcvt_w_d); break;
                        case 0b00001: unwrap_opcall(_rv_fcvt_wu_d); break;
                        case 0b00010: unwrap_opcall(_rv_fcvt_l_d); break;
                        case 0b00011: unwrap_opcall(_rv_fcvt_lu_d); break;
                    }
                    case 0b1101001: switch(rs2) {
                        case 0b00000: unwrap_opcall(_rv_fcvt_d_w); break;
                        case 0b00001: unwrap_opcall(_rv_fcvt_d_wu); break;
                        case 0b00010: unwrap_opcall(_rv_fcvt_d_l); break;
                        case 0b00011: unwrap_opcall(_rv_fcvt_d_lu); break;
                    }
                    case 0b0010001: switch(func3) {
                        case 0b000: unwrap_opcall(_rv_fsgnj_d); break;
                        case 0b001: unwrap_opcall(_rv_fsgnjn_d); break;
                        case 0b010: unwrap_opcall(_rv_fsgnjx_d); break;
                    }
                    case 0b0010101: switch(func3) {
                        case 0b000: unwrap_opcall(_rv_fmin_d); break;
                        case 0b001: unwrap_opcall(_rv_fmax_d); break;
                    }
                    case 0b1010001: switch(func3) {
                        case 0b010: unwrap_opcall(_rv_feq_d); break;
                        case 0b001: unwrap_opcall(_rv_flt_d); break;
                        case 0b000: unwrap_opcall(_rv_fle_d); break;
                    }
                    case 0b1110001: switch(func3) {
                        case 0b001: unwrap_opcall(_rv_fclass_d); break;
                        case 0b000: unwrap_opcall(_rv_fmv_x_d); break;
                    }
                }
                case 0b0101111: switch(func3) {
                    case 0b010: switch(func5) {
                        case 0b00010: unwrap_opcall(_rv_lrw); break;
                        case 0b00011: unwrap_opcall(_rv_scw); break;
                        case 0b00001: unwrap_opcall(_rv_amoswap_w); break;
                        case 0b00000: unwrap_opcall(_rv_amoadd_w); break;
                        case 0b00100: unwrap_opcall(_rv_amoxor_w); break;
                        case 0b01100: unwrap_opcall(_rv_amoand_w); break;
                        case 0b01000: unwrap_opcall(_rv_amoor_w); break;
                        case 0b10000: unwrap_opcall(_rv_amomin_w); break;
                        case 0b10100: unwrap_opcall(_rv_amomax_w); break;
                        case 0b11000: unwrap_opcall(_rv_amominu_w); break;
                        case 0b11100: unwrap_opcall(_rv_amomaxu_w); break;
                    }
                    case 0b011: switch(func5) {
                        case 0b00010: unwrap_opcall(_rv_lrd); break;
                        case 0b00011: unwrap_opcall(_rv_scd); break;
                        case 0b00001: unwrap_opcall(_rv_amoswap_d); break;
                        case 0b00000: unwrap_opcall(_rv_amoadd_d); break;
                        case 0b00100: unwrap_opcall(_rv_amoxor_d); break;
                        case 0b01100: unwrap_opcall(_rv_amoand_d); break;
                        case 0b01000: unwrap_opcall(_rv_amoor_d); break;
                        case 0b10000: unwrap_opcall(_rv_amomin_d); break;
                        case 0b10100: unwrap_opcall(_rv_amomax_d); break;
                        case 0b11000: unwrap_opcall(_rv_amominu_d); break;
                        case 0b11100: unwrap_opcall(_rv_amomaxu_d); break;
                    }
                }
                case 0b0111011: switch(func3) {
                    case 0b000: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_addw); break;
                        case 0b0100000: unwrap_opcall(_rv_subw); break;
                        case 0b0000001: unwrap_opcall(_rv_mulw); break;
                    }
                    case 0b101: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_srlw); break;
                        case 0b0100000: unwrap_opcall(_rv_sraw); break;
                        case 0b0000001: unwrap_opcall(_rv_divuw); break;
                    }
                    case 0b001: unwrap_opcall(_rv_sllw); break;
                    case 0b100: unwrap_opcall(_rv_divw); break;
                    case 0b110: unwrap_opcall(_rv_remw); break;
                    case 0b111: unwrap_opcall(_rv_remuw); break;
                }
                case 0b0110011: switch(func3) {
                    case 0b000: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_add); break;
                        case 0b0100000: unwrap_opcall(_rv_sub); break;
                        case 0b0000001: unwrap_opcall(_rv_mul); break;
                    }
                    case 0b001: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_sll); break;
                        case 0b0000001: unwrap_opcall(_rv_mulh); break;
                    }
                    case 0b010: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_slt); break;
                        case 0b0000001: unwrap_opcall(_rv_mulhsu); break;
                    }
                    case 0b011: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_sltu); break;
                        case 0b0000001: unwrap_opcall(_rv_mulhu); break;
                    }
                    case 0b100: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_xor); break;
                        case 0b0000001: unwrap_opcall(_rv_div); break;
                    }
                    case 0b101: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_srl); break;
                        case 0b0100000: unwrap_opcall(_rv_sra); break;
                        case 0b0000001: unwrap_opcall(_rv_divu); break;
                    }
                    case 0b110: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_or); break;
                        case 0b0000001: unwrap_opcall(_rv_rem); break;
                    }
                    case 0b111: switch(func7) {
                        case 0b0000000: unwrap_opcall(_rv_and); break;
                        case 0b0000001: unwrap_opcall(_rv_remu); break;
                    }
                }
            }
                // S TYPE
            case stype: scr_write(int32_t, imm, _imm_s); switch(opcode7) {
                case 0b0100011: switch(func3) {
                    case 0b000: unwrap_opcall(_rv_sb); break;
                    case 0b001: unwrap_opcall(_rv_sh); break;
                    case 0b010: unwrap_opcall(_rv_sw); break;
                    case 0b011: unwrap_opcall(_rv_sd); break;
                }
                case 0b0100111: switch(func3) {
                    case 0b010: unwrap_opcall(_rv_fsw); break;
                    case 0b011: unwrap_opcall(_rv_fsd); break;
                    // case 0b100: unwrap_opcall(_rv_fsq); break; // TODO:
                }
            }
                // B TYPE
            case btype: scr_write(int32_t, imm, _imm_b); switch(func3) {
                case 0b000: unwrap_opcall(_rv_beq); break;
                case 0b001: unwrap_opcall(_rv_bne); break;
                case 0b100: unwrap_opcall(_rv_blt); break;
                case 0b101: unwrap_opcall(_rv_bge); break;
                case 0b110: unwrap_opcall(_rv_bltu); break;
                case 0b111: unwrap_opcall(_rv_bgeu); break;
            }
                // U TYPE
            case utype: scr_write(int32_t, imm, _imm_u); switch(opcode7) {
                case 0b0110111: unwrap_opcall(_rv_lui); break;
                case 0b0010111: unwrap_opcall(_rv_auipc); break;
            }
                // J TYPE
            case jtype: scr_write(int32_t, imm, _imm_j); switch(opcode7) {
                case 0b1101111: unwrap_opcall(_rv_jal); break;
            }
                // R4 TYPE
            case r4type: {}
        }
    }
};
#endif // VMCODE_H
