#include <windows.h>
#include <stdint.h>

#include "mock.hpp"
#include "vmcs.h"
#include "vmctx.h"
#include "vmmem.h"

namespace rvm64 {
    __function int64_t vm_main(void) {
        vmcs_t vm_instance = { };
        vmcs= &vm_instance;

        rvm64::memory::vm_init(); // initialize the process space. Make sure "read_program" checks boundaries (process->size >= program->size).
        while(!vmcs->halt) {
            if (!read_program_from_packet()) { // continue reading until successful
                continue;
            }
        };

        rvm64::context::vm_entry();
        rvm64::memory::vm_end();

        return vmcs->reason;
    }
};

struct opcode {
    uint8_t mask;
    typenum type;
};
__rdata const opcode encoding[] = {
    { 0b1010011, rtype  }, { 0b1000011, rtype  }, { 0b0110011, rtype  }, { 0b1000111, r4type }, { 0b1001011, r4type }, { 0b1001111, r4type },
    { 0b0000011, itype  }, { 0b0001111, itype  }, { 0b1100111, itype  }, { 0b0010011, itype  }, { 0b1110011, itype  }, { 0b0100011, stype  },
    { 0b0100111, stype  }, { 0b1100011, btype  }, { 0b0010111, utype  }, { 0b0110111, utype  }, { 0b1101111, jtype  },
};

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

    int32_t imm_i = (int32_t)(opcode) >> 20;
    int32_t imm_s = ((int32_t)(((opcode >> 25) << 5) | ((opcode >> 7) & 0x1F))) << 20 >> 20;
    int32_t imm_u = opcode & 0xFFFFF000;

    int32_t imm_b =
    ((opcode >> 31) & 0x1) << 12 |
    ((opcode >> 25) & 0x3F) << 5 |
    ((opcode >> 8) & 0xF) << 1 |
    ((opcode >> 7) & 0x1) << 11;
    imm_b = (int32_t)(imm_b << 19) >> 19;  // Sign-extend from 13 bits

    int32_t imm_j =
    ((opcode >> 31) & 0x1) << 20 |
    ((opcode >> 21) & 0x3FF) << 1 |
    ((opcode >> 20) & 0x1) << 11 |
    ((opcode >> 12) & 0xFF) << 12;
    imm_j = (int32_t)(imm_j << 11) >> 11;  // Sign-extend from 21 bits

    vmcs->vscratch1[0] = func3;
    vmcs->vscratch1[1] = rd;
    vmcs->vscratch1[2] = rs1;
    vmcs->vscratch1[3] = rs2;
    vmcs->vscratch1[4] = rs3;
    vmcs->vscratch1[5] = func7;
    vmcs->vscratch1[6] = func5;
    vmcs->vscratch1[7] = func2;

    vmcs->vscratch2[0] = imm_i;
    vmcs->vscratch2[1] = imm_s;
    vmcs->vscratch2[2] = imm_b;
    vmcs->vscratch2[3] = imm_u;
    vmcs->vscratch2[4] = imm_j;

    switch(decoded) {
        // I_TYPE
        case itype: switch(opcode7) {
            case 0b0010011: switch(func3) {
                case 0b000: { return ((uintptr_t*)vmcs->handler)[_addi];  } // TODO: drop fields into vmcs->vscratch
                case 0b010: { return ((uintptr_t*)vmcs->handler)[_slti];  }
                case 0b011: { return ((uintptr_t*)vmcs->handler)[_sltiu]; }
                case 0b100: { return ((uintptr_t*)vmcs->handler)[_xori];  }
                case 0b110: { return ((uintptr_t*)vmcs->handler)[_ori];   }
                case 0b111: { return ((uintptr_t*)vmcs->handler)[_andi];  }
                case 0b001: { return ((uintptr_t*)vmcs->handler)[_slli];  }
                case 0b101: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_srli];
                    case 0b0100000: return ((uintptr_t*)vmcs->handler)[_srai];
                }
            }
            case 0b0011011: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs->handler)[_addiw];
                case 0b001: return ((uintptr_t*)vmcs->handler)[_slliw];
                case 0b101: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_srliw];
                    case 0b0100000: return ((uintptr_t*)vmcs->handler)[_sraiw];
                }
            }
            case 0b0000011: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs->handler)[_lb];
                case 0b001: return ((uintptr_t*)vmcs->handler)[_lh];
                case 0b010: return ((uintptr_t*)vmcs->handler)[_lw];
                case 0b100: return ((uintptr_t*)vmcs->handler)[_lbu];
                case 0b101: return ((uintptr_t*)vmcs->handler)[_lhu];
                case 0b110: return ((uintptr_t*)vmcs->handler)[_lwu];
                case 0b011: return ((uintptr_t*)vmcs->handler)[_ld];
            }
        } 

        // R_TYPE
        case rtype: switch(opcode7) {
            case 0b1010011: switch(func7) {
                case 0b0000001: return ((uintptr_t*)vmcs->handler)[_fadd_d];
                case 0b0000101: return ((uintptr_t*)vmcs->handler)[_fsub_d];
                case 0b0001001: return ((uintptr_t*)vmcs->handler)[_fmul_d];
                case 0b0001101: return ((uintptr_t*)vmcs->handler)[_fdiv_d];
                case 0b0101101: return ((uintptr_t*)vmcs->handler)[_fsqrt_d];
                case 0b1111001: return ((uintptr_t*)vmcs->handler)[_fmv_d_x];
                case 0b0100000: switch(rs2) {
                    case 0b00001: return ((uintptr_t*)vmcs->handler)[_fcvt_s_d];
                    case 0b00011: return ((uintptr_t*)vmcs->handler)[_fcvt_s_q];
                }
                case 0b0100001: switch(rs2) {
                    case 0b00000: return ((uintptr_t*)vmcs->handler)[_fcvt_d_s];
                    case 0b00011: return ((uintptr_t*)vmcs->handler)[_fcvt_d_q];
                }
                case 0b1100001: switch(rs2) {
                    case 0b00000: return ((uintptr_t*)vmcs->handler)[_fcvt_w_d];
                    case 0b00001: return ((uintptr_t*)vmcs->handler)[_fcvt_wu_d];
                    case 0b00010: return ((uintptr_t*)vmcs->handler)[_fcvt_l_d];
                    case 0b00011: return ((uintptr_t*)vmcs->handler)[_fcvt_lu_d];
                }
                case 0b1101001: switch(rs2) {
                    case 0b00000: return ((uintptr_t*)vmcs->handler)[_fcvt_d_w];
                    case 0b00001: return ((uintptr_t*)vmcs->handler)[_fcvt_d_wu];
                    case 0b00010: return ((uintptr_t*)vmcs->handler)[_fcvt_d_l];
                    case 0b00011: return ((uintptr_t*)vmcs->handler)[_fcvt_d_lu];
                }
                case 0b0010001: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_fsgnj_d];
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_fsgnjn_d];
                    case 0b010: return ((uintptr_t*)vmcs->handler)[_fsgnjx_d];
                }
                case 0b0010101: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_fmin_d];
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_fmax_d];
                }
                case 0b1010001: switch(func3) {
                    case 0b010: return ((uintptr_t*)vmcs->handler)[_feq_d];
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_flt_d];
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_fle_d];
                }
                case 0b1110001: switch(func3) {
                    case 0b001: return ((uintptr_t*)vmcs->handler)[_fclass_d];
                    case 0b000: return ((uintptr_t*)vmcs->handler)[_fmv_x_d];
                }
            }
            case 0b0101111: switch(func3) {
                case 0b010: switch(func5) {
                    case 0b00010: return ((uintptr_t*)vmcs->handler)[_lrw];
                    case 0b00011: return ((uintptr_t*)vmcs->handler)[_scw];
                    case 0b00001: return ((uintptr_t*)vmcs->handler)[_amoswap_w];
                    case 0b00000: return ((uintptr_t*)vmcs->handler)[_amoadd_w];
                    case 0b00100: return ((uintptr_t*)vmcs->handler)[_amoxor_w];
                    case 0b01100: return ((uintptr_t*)vmcs->handler)[_amoand_w];
                    case 0b01000: return ((uintptr_t*)vmcs->handler)[_amoor_w];
                    case 0b10000: return ((uintptr_t*)vmcs->handler)[_amomin_w];
                    case 0b10100: return ((uintptr_t*)vmcs->handler)[_amomax_w];
                    case 0b11000: return ((uintptr_t*)vmcs->handler)[_amominu_w];
                    case 0b11100: return ((uintptr_t*)vmcs->handler)[_amomaxu_w];
                }
                case 0b011: switch(func5) {
                    case 0b00010: return ((uintptr_t*)vmcs->handler)[_lrd];
                    case 0b00011: return ((uintptr_t*)vmcs->handler)[_scd];
                    case 0b00001: return ((uintptr_t*)vmcs->handler)[_amoswap_d];
                    case 0b00000: return ((uintptr_t*)vmcs->handler)[_amoadd_d];
                    case 0b00100: return ((uintptr_t*)vmcs->handler)[_amoxor_d];
                    case 0b01100: return ((uintptr_t*)vmcs->handler)[_amoand_d];
                    case 0b01000: return ((uintptr_t*)vmcs->handler)[_amoor_d];
                    case 0b10000: return ((uintptr_t*)vmcs->handler)[_amomin_d];
                    case 0b10100: return ((uintptr_t*)vmcs->handler)[_amomax_d];
                    case 0b11000: return ((uintptr_t*)vmcs->handler)[_amominu_d];
                    case 0b11100: return ((uintptr_t*)vmcs->handler)[_amomaxu_d];
                }
            }
            case 0b0111011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_addw];
                    case 0b0100000: return ((uintptr_t*)vmcs->handler)[_subw];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_mulw];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_srlw];
                    case 0b0100000: return ((uintptr_t*)vmcs->handler)[_sraw];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_divuw];
                }
                case 0b001: return ((uintptr_t*)vmcs->handler)[_sllw];
                case 0b100: return ((uintptr_t*)vmcs->handler)[_divw];
                case 0b110: return ((uintptr_t*)vmcs->handler)[_remw];
                case 0b111: return ((uintptr_t*)vmcs->handler)[_remuw];
            }
            case 0b0110011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_add];
                    case 0b0100000: return ((uintptr_t*)vmcs->handler)[_sub];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_mul];
                }
                case 0b001: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_sll];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_mulh];
                }
                case 0b010: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_slt];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_mulhsu];
                }
                case 0b011: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_sltu];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_mulhu];
                }
                case 0b100: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_xor];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_div];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_srl];
                    case 0b0100000: return ((uintptr_t*)vmcs->handler)[_sra];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_divu];
                }
                case 0b110: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_or];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_rem];
                }
                case 0b111: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs->handler)[_and];
                    case 0b0000001: return ((uintptr_t*)vmcs->handler)[_remu];
                }
            }
        } 
        // S TYPE
        case stype: switch(opcode7) {
            case 0b0100011: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs->handler)[_sb];
                case 0b001: return ((uintptr_t*)vmcs->handler)[_sh];
                case 0b010: return ((uintptr_t*)vmcs->handler)[_sw];
                case 0b011: return ((uintptr_t*)vmcs->handler)[_sd];
            }
            case 0b0100111: switch(func3) {
                case 0b010: return ((uintptr_t*)vmcs->handler)[_fsw];
                case 0b011: return ((uintptr_t*)vmcs->handler)[_fsd];
                case 0b100: return ((uintptr_t*)vmcs->handler)[_fsq];
            }
        }
        // B TYPE
        case btype: switch(func3) {
            case 0b000: return ((uintptr_t*)vmcs->handler)[_beq];
            case 0b001: return ((uintptr_t*)vmcs->handler)[_bne];
            case 0b100: return ((uintptr_t*)vmcs->handler)[_blt];
            case 0b101: return ((uintptr_t*)vmcs->handler)[_bge];
            case 0b110: return ((uintptr_t*)vmcs->handler)[_bltu];
            case 0b111: return ((uintptr_t*)vmcs->handler)[_bgeu];
        }
        // U TYPE
        case utype: switch(opcode7) {
            case 0b0110111: return ((uintptr_t*)vmcs->handler)[_lui];
            case 0b0010111: return ((uintptr_t*)vmcs->handler)[_auipc];
        }
        // J TYPE
        case jtype: switch(opcode7) {
            case 0b1101111: return ((uintptr_t*)vmcs->handler)[_jal];
        }
        // R4 TYPE
        case r4type: {}
    }
}

