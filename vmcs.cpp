#include <windows.h>
#include <stdint.h>

#include "vmcs.h"
#include "vmctx.h"
#include "vmmem.h"
#include "vmops.h"

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
    uint8_t rs2   = (opcode >> 20) & 0x1F;
    uint8_t func3 = (opcode >> 12) & 0x7;
    uint8_t func7 = (opcode >> 25) & 0x7F;
    uint8_t func5 = (func7 >> 2) & 0x1F;

    // TODO: imm_mask is not defined for itype: case func3(0b101)
    uint16_t imm_11_0 = (opcode >> 20) & 0xFFF;
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
                case 0b000: return rvm64::operation::__handler[_addi];
                case 0b010: return rvm64::operation::__handler[_slti];
                case 0b011: return rvm64::operation::__handler[_sltiu];
                case 0b100: return rvm64::operation::__handler[_xori];
                case 0b110: return rvm64::operation::__handler[_ori];
                case 0b111: return rvm64::operation::__handler[_andi];
                case 0b001: return rvm64::operation::__handler[_slli];
                case 0b101: switch(imm_mask) {
                    case 0b0000000: return rvm64::operation::__handler[_srli];
                    case 0b0100000: return rvm64::operation::__handler[_srai];
                }
            }
            case 0b0011011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[_addiw];
                case 0b001: return rvm64::operation::__handler[_slliw];
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_srliw];
                    case 0b0100000: return rvm64::operation::__handler[_sraiw];
                }
            }
            case 0b0000011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[_lb];
                case 0b001: return rvm64::operation::__handler[_lh];
                case 0b010: return rvm64::operation::__handler[_lw];
                case 0b100: return rvm64::operation::__handler[_lbu];
                case 0b101: return rvm64::operation::__handler[_lhu];
                case 0b110: return rvm64::operation::__handler[_lwu];
                case 0b011: return rvm64::operation::__handler[_ld];
            }
        } 

        // R_TYPE
        case rtype: switch(opcode) {
            case 0b1010011: switch(func7) {
                case 0b0000001: return rvm64::operation::__handler[_fadd_d];
                case 0b0000101: return rvm64::operation::__handler[_fsub_d];
                case 0b0001001: return rvm64::operation::__handler[_fmul_d];
                case 0b0001101: return rvm64::operation::__handler[_fdiv_d];
                case 0b0101101: return rvm64::operation::__handler[_fsqrt_d];
                case 0b1111001: return rvm64::operation::__handler[_fmv_d_x];
                case 0b0100000: switch(rs2) {
                    case 0b00001: return rvm64::operation::__handler[_fcvt_s_d];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_s_q];
                }
                case 0b0100001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[_fcvt_d_s];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_d_q];
                }
                case 0b1100001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[_fcvt_w_d];
                    case 0b00001: return rvm64::operation::__handler[_fcvt_wu_d];
                    case 0b00010: return rvm64::operation::__handler[_fcvt_l_d];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_lu_d];
                }
                case 0b1101001: switch(rs2) {
                    case 0b00000: return rvm64::operation::__handler[_fcvt_d_w];
                    case 0b00001: return rvm64::operation::__handler[_fcvt_d_wu];
                    case 0b00010: return rvm64::operation::__handler[_fcvt_d_l];
                    case 0b00011: return rvm64::operation::__handler[_fcvt_d_lu];
                }
                case 0b0010001: switch(func3) {
                    case 0b000: return rvm64::operation::__handler[_fsgnj_d];
                    case 0b001: return rvm64::operation::__handler[_fsgnjn_d];
                    case 0b010: return rvm64::operation::__handler[_fsgnjx_d];
                }
                case 0b0010101: switch(func3) {
                    case 0b000: return rvm64::operation::__handler[_fmin_d];
                    case 0b001: return rvm64::operation::__handler[_fmax_d];
                }
                case 0b1010001: switch(func3) {
                    case 0b010: return rvm64::operation::__handler[_feq_d];
                    case 0b001: return rvm64::operation::__handler[_flt_d];
                    case 0b000: return rvm64::operation::__handler[_fle_d];
                }
                case 0b1110001: switch(func3) {
                    case 0b001: return rvm64::operation::__handler[_fclass_d];
                    case 0b000: return rvm64::operation::__handler[_fmv_x_d];
                }
            }
            case 0b0101111: switch(func3) {
                case 0b010: switch(func5) {
                    case 0b00010: return rvm64::operation::__handler[_lrw];
                    case 0b00011: return rvm64::operation::__handler[_scw];
                    case 0b00001: return rvm64::operation::__handler[_amoswap_w];
                    case 0b00000: return rvm64::operation::__handler[_amoadd_w];
                    case 0b00100: return rvm64::operation::__handler[_amoxor_w];
                    case 0b01100: return rvm64::operation::__handler[_amoand_w];
                    case 0b01000: return rvm64::operation::__handler[_amoor_w];
                    case 0b10000: return rvm64::operation::__handler[_amomin_w];
                    case 0b10100: return rvm64::operation::__handler[_amomax_w];
                    case 0b11000: return rvm64::operation::__handler[_amominu_w];
                    case 0b11100: return rvm64::operation::__handler[_amomaxu_w];
                }
                case 0b011: switch(func5) {
                    case 0b00010: return rvm64::operation::__handler[_lrd];
                    case 0b00011: return rvm64::operation::__handler[_scd];
                    case 0b00001: return rvm64::operation::__handler[_amoswap_d];
                    case 0b00000: return rvm64::operation::__handler[_amoadd_d];
                    case 0b00100: return rvm64::operation::__handler[_amoxor_d];
                    case 0b01100: return rvm64::operation::__handler[_amoand_d];
                    case 0b01000: return rvm64::operation::__handler[_amoor_d];
                    case 0b10000: return rvm64::operation::__handler[_amomin_d];
                    case 0b10100: return rvm64::operation::__handler[_amomax_d];
                    case 0b11000: return rvm64::operation::__handler[_amominu_d];
                    case 0b11100: return rvm64::operation::__handler[_amomaxu_d];
                }
            }
            case 0b0111011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_addw];
                    case 0b0100000: return rvm64::operation::__handler[_subw];
                    case 0b0000001: return rvm64::operation::__handler[_mulw];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_srlw];
                    case 0b0100000: return rvm64::operation::__handler[_sraw];
                    case 0b0000001: return rvm64::operation::__handler[_divuw];
                }
                case 0b001: return rvm64::operation::__handler[_sllw];
                case 0b100: return rvm64::operation::__handler[_divw];
                case 0b110: return rvm64::operation::__handler[_remw];
                case 0b111: return rvm64::operation::__handler[_remuw];
            }
            case 0b0110011: switch(func3) {
                case 0b000: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_add];
                    case 0b0100000: return rvm64::operation::__handler[_sub];
                    case 0b0000001: return rvm64::operation::__handler[_mul];
                }
                case 0b001: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_sll];
                    case 0b0000001: return rvm64::operation::__handler[_mulh];
                }
                case 0b010: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_slt];
                    case 0b0000001: return rvm64::operation::__handler[_mulhsu];
                }
                case 0b011: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_sltu];
                    case 0b0000001: return rvm64::operation::__handler[_mulhu];
                }
                case 0b100: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_xor];
                    case 0b0000001: return rvm64::operation::__handler[_div];
                }
                case 0b101: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_srl];
                    case 0b0100000: return rvm64::operation::__handler[_sra];
                    case 0b0000001: return rvm64::operation::__handler[_divu];
                }
                case 0b110: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_or];
                    case 0b0000001: return rvm64::operation::__handler[_rem];
                }
                case 0b111: switch(func7) {
                    case 0b0000000: return rvm64::operation::__handler[_and];
                    case 0b0000001: return rvm64::operation::__handler[_remu];
                }
            }
        } 
        // S TYPE
        case stype: switch(opcode) {
            case 0b0100011: switch(func3) {
                case 0b000: return rvm64::operation::__handler[_sb];
                case 0b001: return rvm64::operation::__handler[_sh];
                case 0b010: return rvm64::operation::__handler[_sw];
                case 0b011: return rvm64::operation::__handler[_sd];
            }
            case 0b0100111: switch(func3) {
                case 0b010: return rvm64::operation::__handler[_fsw];
                case 0b011: return rvm64::operation::__handler[_fsd];
                case 0b100: return rvm64::operation::__handler[_fsq];
            }
        }
        // B TYPE
        case btype: switch(func3) {
            case 0b000: return rvm64::operation::__handler[_beq];
            case 0b001: return rvm64::operation::__handler[_bne];
            case 0b100: return rvm64::operation::__handler[_blt];
            case 0b101: return rvm64::operation::__handler[_bge];
            case 0b110: return rvm64::operation::__handler[_bltu];
            case 0b111: return rvm64::operation::__handler[_bgeu];
        }
        // U TYPE
        case utype: switch(opcode) {
            case 0b0110111: return rvm64::operation::__handler[_lui];
            case 0b0010111: return rvm64::operation::__handler[_auipc];
        }
        // J TYPE
        case jtype: switch(opcode) {
            case 0b1101111: return rvm64::operation::__handler[_jal];
        }
        // R4 TYPE
        case r4type: {}
    }
}

