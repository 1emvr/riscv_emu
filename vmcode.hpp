#ifndef VMCODE_H
#define VMCODE_H
#include "vmmain.hpp"
#include "vmcrypt.hpp"

namespace rvm64::decoder {
    struct opcode {
        uint8_t mask;
        typenum type;
    };

    enum handler_index : uint8_t {
	    // ITYPE
	    _rv_addi, _rv_slti, _rv_sltiu, _rv_xori,
	    _rv_ori, _rv_andi, _rv_slli, _rv_srli,
	    _rv_srai, _rv_addiw, _rv_slliw, _rv_srliw,
	    _rv_sraiw, _rv_lb, _rv_lh, _rv_lw,
	    _rv_lbu, _rv_lhu, _rv_lwu, _rv_ld,
	    _rv_flq, _rv_fence, _rv_fence_i, _rv_jalr,
	    _rv_ecall, _rv_ebreak, _rv_csrrw, _rv_csrrs,
	    _rv_csrrc, _rv_csrrwi, _rv_csrrsi, _rv_csrrci,
	    _rv_fclass_d, _rv_lrw, _rv_lrd, _rv_fmv_d_x,
	    _rv_fcvt_s_d, _rv_fcvt_d_s, _rv_fcvt_w_d, _rv_fcvt_wu_d,
	    _rv_fcvt_d_w, _rv_fcvt_d_wu,

	    // RTYPE
	    _rv_fadd_d, _rv_fsub_d, _rv_fmul_d, _rv_fdiv_d,
	    _rv_fsqrt_d, _rv_fsgnj_d, _rv_fsgnjn_d, _rv_fsgnjx_d,
	    _rv_fmin_d, _rv_fmax_d, _rv_feq_d, _rv_flt_d,
	    _rv_fle_d, _rv_scw, _rv_amoswap_w, _rv_amoadd_w,
	    _rv_amoxor_w, _rv_amoand_w, _rv_amoor_w, _rv_amomin_w,
	    _rv_amomax_w, _rv_amominu_w, _rv_amomaxu_w,
	    _rv_scd, _rv_amoswap_d, _rv_amoadd_d,
	    _rv_amoxor_d, _rv_amoand_d, _rv_amoor_d, _rv_amomin_d,
	    _rv_amomax_d, _rv_amominu_d, _rv_amomaxu_d,
	    _rv_addw, _rv_subw, _rv_mulw, _rv_srlw,
	    _rv_sraw, _rv_divuw, _rv_sllw, _rv_divw,
	    _rv_remw, _rv_remuw, _rv_add, _rv_sub,
	    _rv_mul, _rv_sll, _rv_mulh, _rv_slt,
	    _rv_mulhsu, _rv_sltu, _rv_mulhu, _rv_xor,
	    _rv_div, _rv_srl, _rv_sra, _rv_divu,
	    _rv_or, _rv_rem, _rv_and, _rv_remu,

	    // STYPE
	    _rv_sb, _rv_sh, _rv_sw, _rv_sd,
	    _rv_fsw, _rv_fsd,

	    // BTYPE
	    _rv_beq, _rv_bne, _rv_blt, _rv_bge,
	    _rv_bltu, _rv_bgeu,

	    // UTYPE/JTYPE
	    _rv_lui, _rv_auipc, _rv_jal
    };

    // TODO: revise encoding and ratify which operations belong to what class/extension
    _rdata const opcode encoding[] = {
        { 0b1010011, rtype  }, { 0b1000011, rtype  }, { 0b0110011, rtype  }, { 0b1000111, r4type }, { 0b1001011, r4type }, { 0b1001111, r4type },
        { 0b0000011, itype  }, { 0b0001111, itype  }, { 0b1100111, itype  }, { 0b0010011, itype  }, { 0b1110011, itype  }, { 0b0100011, stype  },
        { 0b0100111, stype  }, { 0b1100011, btype  }, { 0b0010111, utype  }, { 0b0110111, utype  }, { 0b1101111, jtype  },
    };

    inline int32_t sign_extend(int32_t val, int bits) {
        int shift = 32 - bits;
        return (val << shift) >> shift;
    }

    inline uint8_t shamt_i(uint32_t opcode) {
        return (opcode >> 20) & 0x1F;
    }

    // NOTE: annoying as fuck to read. just let GPT do the math and say fuck it.
    inline int32_t imm_u(uint32_t opcode) { return (int32_t)opcode & 0xFFFFF000; }
    inline int32_t imm_i(uint32_t opcode) { return (int32_t)sign_extend((opcode >> 20), 12); }
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

	// NOTE: in __vmcall, this will spill to the stack like crazy. consider using normal __stdcall.
    _function void vm_decode(uint32_t opcode) {
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
            vmcs->reason = vm_illegal_op;
            return;
        }

        switch(decoded) {
            case itype:
				{
					uint8_t func3 = (opcode >> 12) & 0x7;
					scr_write(uint8_t, screnum::rs1, (opcode >> 15) & 0x1F);

					switch(opcode7) {
						case 0b0010011: 
							{
								switch(func3) {
									case 0b000: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_addi); break; }
									case 0b010: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_slti); break; }
									case 0b011: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_sltiu); break; }
									case 0b100: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_xori); break; }
									case 0b110: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_ori);  break; }
									case 0b111: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_andi); break; }
									case 0b001: { scr_write(int32_t, imm, shamt_i(opcode)); unwrap_opcall(_rv_slli); break; }
									case 0b101: 
												{
        											uint8_t func7 = (opcode >> 24) & 0x7F;
													switch(func7) {
														case 0b0000000: { scr_write(int32_t, imm, shamt_i(opcode)); unwrap_opcall(_rv_srli); break; }
														case 0b0100000: { scr_write(int32_t, imm, shamt_i(opcode)); unwrap_opcall(_rv_srai); break; }
													}
												}
								}
							}
						case 0b0011011: 
							{
								switch(func3) {
									case 0b000: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_addiw); break; }
									case 0b001: { scr_write(int32_t, imm, shamt_i(opcode)); unwrap_opcall(_rv_slliw); break; }
									case 0b101: 
												{
        											uint8_t func7 = (opcode >> 24) & 0x7F;
													switch(func7) {
														case 0b0000000: { scr_write(int32_t, imm, shamt_i(opcode)); unwrap_opcall(_rv_srliw); break; }
														case 0b0100000: { scr_write(int32_t, imm, shamt_i(opcode)); unwrap_opcall(_rv_sraiw); break; }
													}
												}
								}
							}
						case 0b0000011: 
							{
								switch(func3) {
									case 0b000: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_lb); break;  }
									case 0b001: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_lh); break;  }
									case 0b010: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_lw); break;  }
									case 0b100: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_lbu); break; }
									case 0b101: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_lhu); break; }
									case 0b110: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_lwu); break; }
									case 0b011: { scr_write(int32_t, imm, imm_i(opcode)); unwrap_opcall(_rv_ld); break;  }
								}
							}
						default:
							{
								vmcs->halt = 1;
								vmcs->reason = vm_illegal_op;
								return;
							}
					}
				}

			case rtype: 
				{
					scr_write(uint8_t, screnum::rs2, (opcode >> 20) & 0x1F);
					scr_write(uint8_t, screnum::rs1, (opcode >> 15) & 0x1F);
					scr_write(uint8_t, screnum::rd, (opcode >> 7) & 0x1F);

					switch(opcode7) {
						case 0b1010011: 
							{
        						uint8_t func7 = (opcode >> 24) & 0x7F;
								switch(func7) {
									case 0b0000001: { unwrap_opcall(_rv_fadd_d); break; }
									case 0b0000101: { unwrap_opcall(_rv_fsub_d); break; }
									case 0b0001001: { unwrap_opcall(_rv_fmul_d); break; }
									case 0b0001101: { unwrap_opcall(_rv_fdiv_d); break; }
									case 0b0101101: { unwrap_opcall(_rv_fsqrt_d); break; }
									case 0b1111001: { unwrap_opcall(_rv_fmv_d_x); break; }
									case 0b0100000: 
													{
														uint8_t fcvt_mask = (opcode >> 20) & 0x1F;
														switch(fcvt_mask) {
															case 0b00001: unwrap_opcall(_rv_fcvt_s_d); break;
														}
													}
									case 0b0100001: 
													{
														uint8_t fcvt_mask = (opcode >> 20) & 0x1F;
														switch(fcvt_mask) {
															case 0b00000: unwrap_opcall(_rv_fcvt_d_s); break;
														}
													}
									case 0b1100001: 
													{
														uint8_t fcvt_mask = (opcode >> 20) & 0x1F;
														switch(fcvt_mask) {
															case 0b00000: { unwrap_opcall(_rv_fcvt_w_d); break; }
															case 0b00001: { unwrap_opcall(_rv_fcvt_wu_d); break; }
														}
													}
									case 0b1101001: 
													{
														uint8_t fcvt_mask = (opcode >> 20) & 0x1F;
														switch(fcvt_mask) {
															case 0b00000: { unwrap_opcall(_rv_fcvt_d_w); break; }
															case 0b00001: { unwrap_opcall(_rv_fcvt_d_wu); break; }
														}
													}
									case 0b0010001: 
													{
        												uint8_t func3 = (opcode >> 12) & 0x7;
														switch(func3) {
															case 0b000: { unwrap_opcall(_rv_fsgnj_d); break; }
															case 0b001: { unwrap_opcall(_rv_fsgnjn_d); break; }
															case 0b010: { unwrap_opcall(_rv_fsgnjx_d); break; }
														}
													}
									case 0b0010101: 
													{
        												uint8_t func3 = (opcode >> 12) & 0x7;
														switch(func3) {
															case 0b000: { unwrap_opcall(_rv_fmin_d); break; }
															case 0b001: { unwrap_opcall(_rv_fmax_d); break; }
														}
													}
									case 0b1010001: 
													{ 
        												uint8_t func3 = (opcode >> 12) & 0x7;
														switch(func3) {
															case 0b010: { unwrap_opcall(_rv_feq_d); break; }
															case 0b001: { unwrap_opcall(_rv_flt_d); break; }
															case 0b000: { unwrap_opcall(_rv_fle_d); break; }
														}
													}
									case 0b1110001: 
													{
        												uint8_t func3 = (opcode >> 12) & 0x7;
														switch(func3) {
															case 0b001: { unwrap_opcall(_rv_fclass_d); break; }
														}
													}
								}
							}

						case 0b0101111: 
							{
        						uint8_t func5 = (func7 >> 2) & 0x1F;
        						uint8_t func3 = (opcode >> 12) & 0x7;

								switch(func3) {
									case 0b010: 
										{
											switch(func5) {
												case 0b00010: { unwrap_opcall(_rv_lrw); break; }
												case 0b00011: { unwrap_opcall(_rv_scw); break; }
												case 0b00001: { unwrap_opcall(_rv_amoswap_w); break; }
												case 0b00000: { unwrap_opcall(_rv_amoadd_w); break; }
												case 0b00100: { unwrap_opcall(_rv_amoxor_w); break; }
												case 0b01100: { unwrap_opcall(_rv_amoand_w); break; }
												case 0b01000: { unwrap_opcall(_rv_amoor_w); break; }
												case 0b10000: { unwrap_opcall(_rv_amomin_w); break; }
												case 0b10100: { unwrap_opcall(_rv_amomax_w); break; }
												case 0b11000: { unwrap_opcall(_rv_amominu_w); break; }
												case 0b11100: { unwrap_opcall(_rv_amomaxu_w); break; }
											}
										}
									case 0b011: 
										{
											switch(func5) {
												case 0b00010: { unwrap_opcall(_rv_lrd); break; }
												case 0b00011: { unwrap_opcall(_rv_scd); break; }
												case 0b00001: { unwrap_opcall(_rv_amoswap_d); break; }
												case 0b00000: { unwrap_opcall(_rv_amoadd_d); break; }
												case 0b00100: { unwrap_opcall(_rv_amoxor_d); break; }
												case 0b01100: { unwrap_opcall(_rv_amoand_d); break; }
												case 0b01000: { unwrap_opcall(_rv_amoor_d); break; }
												case 0b10000: { unwrap_opcall(_rv_amomin_d); break; }
												case 0b10100: { unwrap_opcall(_rv_amomax_d); break; }
												case 0b11000: { unwrap_opcall(_rv_amominu_d); break; }
												case 0b11100: { unwrap_opcall(_rv_amomaxu_d); break; }
											}
										}
								}
							}

						case 0b0111011: 
							{
        						uint8_t func7 = (opcode >> 24) & 0x7F;
        						uint8_t func3 = (opcode >> 12) & 0x7;

								switch(func3) {
									case 0b000: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_addw); break; }
												case 0b0100000: { unwrap_opcall(_rv_subw); break; }
												case 0b0000001: { unwrap_opcall(_rv_mulw); break; }
											}
										}
									case 0b101: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_srlw); break; }
												case 0b0100000: { unwrap_opcall(_rv_sraw); break; }
												case 0b0000001: { unwrap_opcall(_rv_divuw); break; }
											}
										}
									case 0b001: { unwrap_opcall(_rv_sllw); break; }
									case 0b100: { unwrap_opcall(_rv_divw); break; }
									case 0b110: { unwrap_opcall(_rv_remw); break; }
									case 0b111: { unwrap_opcall(_rv_remuw); break; }
								}
							}
							
						case 0b0110011: 
							{
        						uint8_t func7 = (opcode >> 24) & 0x7F;
        						uint8_t func3 = (opcode >> 12) & 0x7;

								switch(func3) {
									case 0b000: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_add); break; }
												case 0b0100000: { unwrap_opcall(_rv_sub); break; }
												case 0b0000001: { unwrap_opcall(_rv_mul); break; }
											}
										}
									case 0b001: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_sll); break; }
												case 0b0000001: { unwrap_opcall(_rv_mulh); break; }
											}
										}
									case 0b010: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_slt); break; }
												case 0b0000001: { unwrap_opcall(_rv_mulhsu); break; }
											}
										}
									case 0b011: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_sltu); break; }
												case 0b0000001: { unwrap_opcall(_rv_mulhu); break; }
											}
										}
									case 0b100: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_xor); break; }
												case 0b0000001: { unwrap_opcall(_rv_div); break; }
											}
										}
									case 0b101: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_srl); break; }
												case 0b0100000: { unwrap_opcall(_rv_sra); break; }
												case 0b0000001: { unwrap_opcall(_rv_divu); break; }
											}
										}
									case 0b110: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_or); break; }
												case 0b0000001: { unwrap_opcall(_rv_rem); break; }
											}
										}
									case 0b111: 
										{
											switch(func7) {
												case 0b0000000: { unwrap_opcall(_rv_and); break; }
												case 0b0000001: { unwrap_opcall(_rv_remu); break; }
											}
										}
								}
							}
					}
				}

			case stype: 
				{
					uint8_t func3 = (opcode >> 12) & 0x7;

					scr_write(uint8_t, screnum::rs2, (opcode >> 20) & 0x1F);
					scr_write(uint8_t, screnum::rs1, (opcode >> 15) & 0x1F);
					scr_write(int32_t, imm, imm_s(opcode));

					switch(opcode7) {
						case 0b0100011: 
							{
								switch(func3) {
									case 0b000: { unwrap_opcall(_rv_sb); break; }
									case 0b001: { unwrap_opcall(_rv_sh); break; }
									case 0b010: { unwrap_opcall(_rv_sw); break; }
									case 0b011: { unwrap_opcall(_rv_sd); break; }
								}
							}
						case 0b0100111: 
							{
								switch(func3) {
									case 0b010: { unwrap_opcall(_rv_fsw); break; }
									case 0b011: { unwrap_opcall(_rv_fsd); break; }
								}
							}
					}
				}

            case btype: 
				{
					uint8_t func3 = (opcode >> 12) & 0x7;

					scr_write(uint8_t, screnum::rs2, (opcode >> 20) & 0x1F);
					scr_write(uint8_t, screnum::rs1, (opcode >> 15) & 0x1F);
					scr_write(int32_t, imm, imm_b(opcode)); 

					switch(func3) {
						case 0b000: { unwrap_opcall(_rv_beq); break; }
						case 0b001: { unwrap_opcall(_rv_bne); break; }
						case 0b100: { unwrap_opcall(_rv_blt); break; }
						case 0b101: { unwrap_opcall(_rv_bge); break; }
						case 0b110: { unwrap_opcall(_rv_bltu); break; }
						case 0b111: { unwrap_opcall(_rv_bgeu); break; }
					}
				}

            case utype: 
				{
					scr_write(uint8_t, screnum::rd, (opcode >> 7) & 0x1F);
					scr_write(int32_t, screnum::imm, imm_u(opcode)); 

					switch(opcode7) {
						case 0b0110111: { unwrap_opcall(_rv_lui); break; }
						case 0b0010111: { unwrap_opcall(_rv_auipc); break; }
					}
				}

            case jtype: 
				{
					scr_write(uint8_t, screnum::rd, (opcode >> 7) & 0x1F);
					scr_write(int32_t, screnum::imm, imm_j(opcode)); 

					switch(opcode7) {
						case 0b1101111: { unwrap_opcall(_rv_jal); break; }
					}
				}
                // R4 TYPE
			case r4type: 
				{
				}
        }
    }
};
#endif // VMCODE_H
