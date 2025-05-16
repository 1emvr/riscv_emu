#include <windows.h>
#include <stdint.h>

#include "vmcs.h"
#include "vmctx.h"
#include "vmmem.h"

__data hexane *ctx = { };
__data vmcs_t vmcs = { };
__data uintptr_t __stack_cookie = { };

namespace rvm64 {
    __function int64_t vm_main(void) {
        while(!vmcs.halt) {
            if (read_program_from_packet(vmcs.program)) {
                break;
            }
        };

        rvm64::memory::vm_init();
        rvm64::context::vm_entry();
        rvm64::memory::vm_end();
        return vmcs.reason;
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
    // TODO: drop all possible bitfields at the same time and write to scratch upon decoding
    // scratch space can be used for bitfields, immediates, arguments, etc.
    uint8_t func7 = (opcode >> 25) & 0x7F;
    uint8_t rs2   = (opcode >> 20) & 0x1F;
    uint8_t rs1   = (opcode >> 15) & 0x1F;
    uint8_t func3 = (opcode >> 12) & 0x7;
    uint8_t func5 = (func7 >> 2) & 0x1F;

    uint16_t imm_11_0 = (opcode >> 20) & 0xFFF;
    // TODO: other immx:x:x from all xtype instructions

    uint8_t decoded = 0;

    opcode &= 0b1111111; 
    for (int idx = 0; idx < sizeof(encoding); idx++) {
        if (encoding[idx].mask == opcode) {
            decoded = encoding[idx].type;
            break;
        }
    }
    if (!decoded) {
        vmcs.halt = 1;
        vmcs.reason = illegal_op;
        return -1;
    }

    switch(decoded) { 
        // I_TYPE
        case itype: switch(opcode) {
            case 0b0010011: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs.handler)[_addi];
                case 0b010: return ((uintptr_t*)vmcs.handler)[_slti];
                case 0b011: return ((uintptr_t*)vmcs.handler)[_sltiu];
                case 0b100: return ((uintptr_t*)vmcs.handler)[_xori];
                case 0b110: return ((uintptr_t*)vmcs.handler)[_ori];
                case 0b111: return ((uintptr_t*)vmcs.handler)[_andi];
                case 0b001: return ((uintptr_t*)vmcs.handler)[_slli];
                case 0b101: switch(imm_mask) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_srli];
                    case 0b0100000: return ((uintptr_t*)vmcs.handler)[_srai];
                }
            }
            case 0b0011011: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs.handler)[_addiw];
                case 0b001: return ((uintptr_t*)vmcs.handler)[_slliw];
                case 0b101: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_srliw];
                    case 0b0100000: return ((uintptr_t*)vmcs.handler)[_sraiw];
                }
            }
            case 0b0000011: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs.handler)[_lb];
                case 0b001: return ((uintptr_t*)vmcs.handler)[_lh];
                case 0b010: return ((uintptr_t*)vmcs.handler)[_lw];
                case 0b100: return ((uintptr_t*)vmcs.handler)[_lbu];
                case 0b101: return ((uintptr_t*)vmcs.handler)[_lhu];
                case 0b110: return ((uintptr_t*)vmcs.handler)[_lwu];
                case 0b011: return ((uintptr_t*)vmcs.handler)[_ld];
            }
        } 

        // R_TYPE
        case rtype: switch(opcode) {
            case 0b1010011: switch(func7) {
                case 0b0000001: return ((uintptr_t*)vmcs.handler)[_fadd_d];
                case 0b0000101: return ((uintptr_t*)vmcs.handler)[_fsub_d];
                case 0b0001001: return ((uintptr_t*)vmcs.handler)[_fmul_d];
                case 0b0001101: return ((uintptr_t*)vmcs.handler)[_fdiv_d];
                case 0b0101101: return ((uintptr_t*)vmcs.handler)[_fsqrt_d];
                case 0b1111001: return ((uintptr_t*)vmcs.handler)[_fmv_d_x];
                case 0b0100000: switch(rs2) {
                    case 0b00001: return ((uintptr_t*)vmcs.handler)[_fcvt_s_d];
                    case 0b00011: return ((uintptr_t*)vmcs.handler)[_fcvt_s_q];
                }
                case 0b0100001: switch(rs2) {
                    case 0b00000: return ((uintptr_t*)vmcs.handler)[_fcvt_d_s];
                    case 0b00011: return ((uintptr_t*)vmcs.handler)[_fcvt_d_q];
                }
                case 0b1100001: switch(rs2) {
                    case 0b00000: return ((uintptr_t*)vmcs.handler)[_fcvt_w_d];
                    case 0b00001: return ((uintptr_t*)vmcs.handler)[_fcvt_wu_d];
                    case 0b00010: return ((uintptr_t*)vmcs.handler)[_fcvt_l_d];
                    case 0b00011: return ((uintptr_t*)vmcs.handler)[_fcvt_lu_d];
                }
                case 0b1101001: switch(rs2) {
                    case 0b00000: return ((uintptr_t*)vmcs.handler)[_fcvt_d_w];
                    case 0b00001: return ((uintptr_t*)vmcs.handler)[_fcvt_d_wu];
                    case 0b00010: return ((uintptr_t*)vmcs.handler)[_fcvt_d_l];
                    case 0b00011: return ((uintptr_t*)vmcs.handler)[_fcvt_d_lu];
                }
                case 0b0010001: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs.handler)[_fsgnj_d];
                    case 0b001: return ((uintptr_t*)vmcs.handler)[_fsgnjn_d];
                    case 0b010: return ((uintptr_t*)vmcs.handler)[_fsgnjx_d];
                }
                case 0b0010101: switch(func3) {
                    case 0b000: return ((uintptr_t*)vmcs.handler)[_fmin_d];
                    case 0b001: return ((uintptr_t*)vmcs.handler)[_fmax_d];
                }
                case 0b1010001: switch(func3) {
                    case 0b010: return ((uintptr_t*)vmcs.handler)[_feq_d];
                    case 0b001: return ((uintptr_t*)vmcs.handler)[_flt_d];
                    case 0b000: return ((uintptr_t*)vmcs.handler)[_fle_d];
                }
                case 0b1110001: switch(func3) {
                    case 0b001: return ((uintptr_t*)vmcs.handler)[_fclass_d];
                    case 0b000: return ((uintptr_t*)vmcs.handler)[_fmv_x_d];
                }
            }
            case 0b0101111: switch(func3) {
                case 0b010: switch(func5) {
                    case 0b00010: return ((uintptr_t*)vmcs.handler)[_lrw];
                    case 0b00011: return ((uintptr_t*)vmcs.handler)[_scw];
                    case 0b00001: return ((uintptr_t*)vmcs.handler)[_amoswap_w];
                    case 0b00000: return ((uintptr_t*)vmcs.handler)[_amoadd_w];
                    case 0b00100: return ((uintptr_t*)vmcs.handler)[_amoxor_w];
                    case 0b01100: return ((uintptr_t*)vmcs.handler)[_amoand_w];
                    case 0b01000: return ((uintptr_t*)vmcs.handler)[_amoor_w];
                    case 0b10000: return ((uintptr_t*)vmcs.handler)[_amomin_w];
                    case 0b10100: return ((uintptr_t*)vmcs.handler)[_amomax_w];
                    case 0b11000: return ((uintptr_t*)vmcs.handler)[_amominu_w];
                    case 0b11100: return ((uintptr_t*)vmcs.handler)[_amomaxu_w];
                }
                case 0b011: switch(func5) {
                    case 0b00010: return ((uintptr_t*)vmcs.handler)[_lrd];
                    case 0b00011: return ((uintptr_t*)vmcs.handler)[_scd];
                    case 0b00001: return ((uintptr_t*)vmcs.handler)[_amoswap_d];
                    case 0b00000: return ((uintptr_t*)vmcs.handler)[_amoadd_d];
                    case 0b00100: return ((uintptr_t*)vmcs.handler)[_amoxor_d];
                    case 0b01100: return ((uintptr_t*)vmcs.handler)[_amoand_d];
                    case 0b01000: return ((uintptr_t*)vmcs.handler)[_amoor_d];
                    case 0b10000: return ((uintptr_t*)vmcs.handler)[_amomin_d];
                    case 0b10100: return ((uintptr_t*)vmcs.handler)[_amomax_d];
                    case 0b11000: return ((uintptr_t*)vmcs.handler)[_amominu_d];
                    case 0b11100: return ((uintptr_t*)vmcs.handler)[_amomaxu_d];
                }
            }
            case 0b0111011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_addw];
                    case 0b0100000: return ((uintptr_t*)vmcs.handler)[_subw];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_mulw];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_srlw];
                    case 0b0100000: return ((uintptr_t*)vmcs.handler)[_sraw];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_divuw];
                }
                case 0b001: return ((uintptr_t*)vmcs.handler)[_sllw];
                case 0b100: return ((uintptr_t*)vmcs.handler)[_divw];
                case 0b110: return ((uintptr_t*)vmcs.handler)[_remw];
                case 0b111: return ((uintptr_t*)vmcs.handler)[_remuw];
            }
            case 0b0110011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_add];
                    case 0b0100000: return ((uintptr_t*)vmcs.handler)[_sub];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_mul];
                }
                case 0b001: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_sll];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_mulh];
                }
                case 0b010: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_slt];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_mulhsu];
                }
                case 0b011: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_sltu];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_mulhu];
                }
                case 0b100: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_xor];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_div];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_srl];
                    case 0b0100000: return ((uintptr_t*)vmcs.handler)[_sra];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_divu];
                }
                case 0b110: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_or];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_rem];
                }
                case 0b111: switch(func7) {
                    case 0b0000000: return ((uintptr_t*)vmcs.handler)[_and];
                    case 0b0000001: return ((uintptr_t*)vmcs.handler)[_remu];
                }
            }
        } 
        // S TYPE
        case stype: switch(opcode) {
            case 0b0100011: switch(func3) {
                case 0b000: return ((uintptr_t*)vmcs.handler)[_sb];
                case 0b001: return ((uintptr_t*)vmcs.handler)[_sh];
                case 0b010: return ((uintptr_t*)vmcs.handler)[_sw];
                case 0b011: return ((uintptr_t*)vmcs.handler)[_sd];
            }
            case 0b0100111: switch(func3) {
                case 0b010: return ((uintptr_t*)vmcs.handler)[_fsw];
                case 0b011: return ((uintptr_t*)vmcs.handler)[_fsd];
                case 0b100: return ((uintptr_t*)vmcs.handler)[_fsq];
            }
        }
        // B TYPE
        case btype: switch(func3) {
            case 0b000: return ((uintptr_t*)vmcs.handler)[_beq];
            case 0b001: return ((uintptr_t*)vmcs.handler)[_bne];
            case 0b100: return ((uintptr_t*)vmcs.handler)[_blt];
            case 0b101: return ((uintptr_t*)vmcs.handler)[_bge];
            case 0b110: return ((uintptr_t*)vmcs.handler)[_bltu];
            case 0b111: return ((uintptr_t*)vmcs.handler)[_bgeu];
        }
        // U TYPE
        case utype: switch(opcode) {
            case 0b0110111: return ((uintptr_t*)vmcs.handler)[_lui];
            case 0b0010111: return ((uintptr_t*)vmcs.handler)[_auipc];
        }
        // J TYPE
        case jtype: switch(opcode) {
            case 0b1101111: return ((uintptr_t*)vmcs.handler)[_jal];
        }
        // R4 TYPE
        case r4type: {}
    }
}

