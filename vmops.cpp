#include "vmmain.hpp"
#include "vmmem.h"
#include "vmcs.h"

namespace rvm64::operation {
    bool is_nan(double x) {
        union {
            double d;
            uint64_t u;
        } converter;

        converter.d = x;

        uint64_t exponent = converter.u & EXPONENT_MASK;
        uint64_t fraction = converter.u & FRACTION_MASK;

        return (exponent == EXPONENT_MASK) && (fraction != 0);
    }

    // i-type
    namespace itype {
        __function void rv_lrw(uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t value = 0;

            reg_read(uintptr_t, rs1, address);
            mem_read(int32_t, address, value);

            reg_write(int64_t, rd, value);
            rvm64::memory::vm_set_load_rsv(address);
        }

        __function void rv_lrd(uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t value = 0;

            reg_read(uintptr_t, rs1, address);
            rvm64::memory::vm_set_load_rsv(address);

            mem_read(int64_t, address, value);
            reg_write(int64_t, rd, value);
        }

        __function void rv_fmv_d_x(uint8_t rs1, uint8_t rd) {
            int64_t v1 = 0;

            reg_read(int64_t, rs1, v1);
            reg_write(int64_t, rd, v1);
        }

        __function void rv_fcvt_s_d(uint8_t rs1, uint8_t rd) {
            float v1 = 0;

            reg_read(double, rs1, v1);
            reg_write(float, rd, v1);
        }

        __function void rv_fcvt_d_s(uint8_t rs1, uint8_t rd) {
            double v1 = 0;

            reg_read(float, rs1, v1);
            reg_write(double, rd, v1);
        }

        __function void rv_fcvt_w_d(uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0;

            reg_read(double, rs1, v1);
            reg_write(int32_t, rd, v1);
        }

        __function void rv_fcvt_wu_d(uint8_t rs1, uint8_t rd) {
            uint32_t v1 = 0;

            reg_read(double, rs1, v1);
            reg_write(uint32_t, rd, v1);
        }

        __function void rv_fcvt_d_w(uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0;

            reg_read(int32_t, rs1, v1);
            reg_write(double, rd, v1);
        }

        __function void rv_fcvt_d_wu(uint8_t rs1, uint8_t rd) {
            uint32_t v1 = 0;

            reg_read(uint32_t, rs1, v1);
            reg_write(double, rd, v1);
        }

        __function void rv_fclass_d(uint8_t rs1, uint8_t rd) {
            double v1 = 0;
            union {
                double d;
                int64_t u;
            } converter;

            reg_read(double, rs1, v1);
            converter.d = v1;

            uint64_t exponent = (converter.u >> 52) & 0x7FF;
            uint64_t fraction = converter.u & 0xFFFFFFFFFFFFF;
            uint64_t sign = converter.u >> 63;

            if (exponent == 0x7FF) {
                if (fraction == 0) {
                    if (sign == 0) {
                        reg_write(int, rd, 0x7); // +inf
                    } else {
                        reg_write(int, rd, 0x0); // -inf
                    }
                } else {
                    if (fraction & (1LL << 51)) {
                        reg_write(int, rd, 0x8); // quiet NaN
                    } else {
                        reg_write(int, rd, 0x9); // signaling NaN
                    }
                }
            } else if (exponent == 0) {
                if (fraction == 0) {
                    if (sign == 0) {
                        reg_write(int, rd, 0x4); // +0
                    } else {
                        reg_write(int, rd, 0x3); // -0
                    }
                } else {
                    if (sign == 0) {
                        reg_write(int, rd, 0x5); // +subnormal
                    } else {
                        reg_write(int, rd, 0x2); // -subnormal
                    }
                }
            } else {
                if (sign == 0) {
                    reg_write(int, rd, 0x6); // +normal
                } else {
                    reg_write(int, rd, 0x1); // -normal
                }
            }
        }
    };

    // r-type
    namespace rtype {
        __function void rv_scw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t word = 0;

            mem_read(uintptr_t, rs1, address);
            reg_read(int32_t, rs2, word);

            if (rvm64::memory::vm_check_load_rsv(address)) {
                mem_write(int32_t, address, word);
                reg_write(int32_t, rd, 0);

                rvm64::memory::vm_clear_load_rsv();
            } else {
                reg_write(int32_t, rd, 1);
            }
        }

        __function void rv_scd(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int64_t value = 0;
            uintptr_t address = 0;

            reg_read(uintptr_t, rs1, address);
            reg_read(int64_t, rs2, value);

            if (!rvm64::memory::vm_check_load_rsv(address)) {
                reg_write(int64_t, rd, 1);
                return;
            }

            mem_write(int64_t, address, value);
            rvm64::memory::vm_clear_load_rsv();
        }

        __function void rv_fadd_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            float v1 = 0, v2 = 0;

            reg_read(float, rs1, v1);
            reg_read(float, rs2, v2);
            reg_write(float, rd, (v1 + v2));
        }

        __function void rv_fsub_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            float v1 = 0, v2 = 0;

            reg_read(float, rs1, v1);
            reg_read(float, rs2, v2);
            reg_write(float, rd, (v1 - v2));
        }

        __function void rv_fmul_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            float v1 = 0, v2 = 0;

            reg_read(float, rs1, v1);
            reg_read(float, rs2, v2);
            reg_write(float, rd, (v1 * v2));
        }

        __function void rv_fdiv_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            float v1 = 0, v2 = 0;

            reg_read(float, rs1, v1);
            reg_read(float, rs2, v2);
            reg_write(float, rd, (v1 / v2));
        }


        __function void rv_fsgnj_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int64_t v1 = 0, v2 = 0;

            reg_read(int64_t, rs1, v1);
            reg_read(int64_t, rs2, v2);

            int64_t s2 = (v2 >> 63) & 1;

            v1 &= ~(1LL << 63);
            v1 |= (s2 << 63);

            reg_write(int64_t, rd, v1);
        }

        __function void rv_fsgnjn_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int64_t v1 = 0, v2 = 0;

            reg_read(int64_t, rs1, v1);
            reg_read(int64_t, rs2, v2);

            int64_t s2 = ((v2 >> 63) & 1) ^ 1;

            v1 &= ~(1LL << 63);
            v1 |= (s2 << 63);

            reg_write(int64_t, rd, v1);
        }

        __function void rv_fsgnjx_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int64_t v1 = 0, v2 = 0;

            reg_read(int64_t, rs1, v1);
            reg_read(int64_t, rs2, v2);

            int64_t s1 = (v1 >> 63) & 1;
            int64_t s2 = (v2 >> 63) & 1;

            v1 &= ~(1LL << 63);
            v1 |= ((s1 ^ s2) << 63);

            reg_write(int64_t, rd, v1);
        }

        __function void rv_fmin_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            double v1 = 0, v2 = 0;

            reg_read(double, rs1, v1);
            reg_read(double, rs2, v2);

            if (is_nan(v1)) {
                reg_write(double, rd, v2);
            } else if (is_nan(v2)) {
                reg_write(double, rd, v1);
            } else {
                reg_write(double, rd, (v1 > v2) ? v2 : v1);
            }
        }

        __function void rv_fmax_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            double v1 = 0, v2 = 0;

            reg_read(double, rs1, v1);
            reg_read(double, rs2, v2);

            if (is_nan(v1)) {
                reg_write(double, rd, v2);
            } else if (is_nan(v2)) {
                reg_write(double, rd, v1);
            } else {
                reg_write(double, rd, (v1 > v2) ? v1 : v2);
            }
        }

        __function void rv_feq_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            double v1 = 0, v2 = 0;

            reg_read(double, rs1, v1);
            reg_read(double, rs2, v2);

            if (is_nan(v1) || is_nan(v2)) {
                reg_write(bool, rd, false);
                return;
            }

            reg_write(bool, rd, (v1 == v2));
        }

        __function void rv_flt_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            double v1 = 0, v2 = 0;

            reg_read(double, rs1, v1);
            reg_read(double, rs2, v2);

            if (is_nan(v1) || is_nan(v2)) {
                reg_write(bool, rd, false);
                return;
            }

            reg_write(bool, rd, (v1 < v2));
        }

        __function void rv_fle_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            double v1 = 0, v2 = 0;

            reg_read(double, rs1, v1);
            reg_read(double, rs2, v2);

            if (is_nan(v1) || is_nan(v2)) {
                reg_write(bool, rd, false);
                return;
            }

            reg_write(bool, rd, (v1 <= v2));
        }

        __function void rv_addw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int64_t, rd, (int64_t)(v1 + v2));
        }

        __function void rv_subw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int64_t, rd, (int64_t)(v1 - v2));
        }

        __function void rv_mulw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int64_t, rd, (int64_t)(v1 * v2));
        }

        __function void rv_srlw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);

            int shamt = v2 & 0x1F;
            reg_write(int32_t, rd, (v1 >> shamt));
        }

        __function void rv_sraw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);

            uint8_t shamt = v2 & 0x1F;
            reg_write(int32_t, rd, (v1 >> shamt));
        }

        __function void rv_divuw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uint32_t v1 = 0, v2 = 0;

            reg_read(uint32_t, rs1, v1);
            reg_read(uint32_t, rs2, v2);
            reg_write(uint32_t, rd, (v1 / v2));
        }

        __function void rv_sllw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);

            int shamt = v2 & 0x1F;
            reg_write(int32_t, rd, (v1 << shamt));
        }

        __function void rv_divw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int32_t, rd, (v1 / v2));
        }

        __function void rv_remw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int32_t, rd, (v1 % v2));
        }

        __function void rv_remuw(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uint32_t v1 = 0, v2 = 0;

            reg_read(uint32_t, rs1, v1);
            reg_read(uint32_t, rs2, v2);
            reg_write(uint32_t, rd, (v1 % v2));
        }

        __function void rv_add(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int32_t, rd, (v1 + v2));
        }

        __function void rv_sub(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int32_t, rd, (v1 - v2));
        }

        __function void rv_mul(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);
            reg_write(int32_t, rd, (v1 * v2));
        }

        __function void rv_sll(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            int32_t v1 = 0, v2 = 0;

            reg_read(int32_t, rs1, v1);
            reg_read(int32_t, rs2, v2);

            int shamt = v2 & 0x1F;
            reg_write(int32_t, rd, (v1 << shamt));
        }

        __function void rv_mulh(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0, v2 = 0;

            reg_read(intptr_t, rs1, v1);
            reg_read(intptr_t, rs2, v2);

#if UINTPTR_MAX == 0xFFFFFFFF
    int64_t result = (int64_t)(int32_t)v1 * (int64_t)(int32_t)v2;
    reg_write(int32_t, rd, (result >> 32));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
            __int128 result = (__int128) (int64_t) v1 * (__int128) (int64_t) v2;
            reg_write(int64_t, rd, (result >> 64));

#endif
        }

        __function void rv_slt(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0, v2 = 0;

            reg_read(intptr_t, rs1, v1);
            reg_read(intptr_t, rs2, v2);
            reg_write(intptr_t, rd, ((v1 < v2) ? 1 : 0));
        }

        __function void rv_mulhsu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0;
            uintptr_t v2 = 0;

            reg_read(intptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);

#if UINTPTR_MAX == 0xFFFFFFFF
    int64_t result = (int64_t)(int32_t)v1 * (uint64_t)(uint32_t)v2;
    reg_write(int32_t, rd, (result >> 32));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
            __int128 result = (__int128) (int64_t) v1 * (__uint128_t) (uint64_t) v2;
            reg_write(int64_t, rd, (result >> 64));

#endif
        }

        __function void rv_sltu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);
            reg_write(uintptr_t, rd, ((v1 < v2) ? 1 : 0));
        }

        __function void rv_mulhu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);

            uintptr_t result = v1 * v2;
#if UINTPTR_MAX == 0xFFFFFFFF
    reg_write(uintptr_t, rd, (result >> 16));

#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
            reg_write(uintptr_t, rd, (result >> 32));
#endif
        }

        __function void rv_xor(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);
            reg_write(uintptr_t, rd, (v1 ^ v2));
        }

        __function void rv_div(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);
            reg_write(uintptr_t, rd, (v1 / v2));
        }

        __function void rv_srl(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);

            uintptr_t shamt = (v2 & 0x1F);
            reg_write(uintptr_t, rd, (v1 >> shamt));
        }

        __function void rv_sra(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0;
            uint8_t v2 = 0;

            reg_read(intptr_t, rs1, v1);
            reg_read(uint8_t, rs2, v2);

            uint8_t shamt = (v2 & 0x1F);
            reg_write(intptr_t, rd, (v1 >> shamt));
        }

        __function void rv_divu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);

            if (v2 == 0) {
                reg_write(uintptr_t, rd, 0);
            } else {
                reg_write(uintptr_t, rd, (v1 / v2));
            }
        }

        __function void rv_or(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0, v2 = 0;

            reg_read(intptr_t, rs1, v1);
            reg_read(intptr_t, rs2, v2);
            reg_write(intptr_t, rd, (v1 | v2));
        }

        __function void rv_rem(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            intptr_t v1 = 0, v2 = 0;

            reg_read(intptr_t, rs1, v1);
            reg_read(intptr_t, rs2, v2);

            if (v2 == 0) {
                reg_write(intptr_t, rd, 0);
            } else {
                reg_write(intptr_t, rd, (v1 % v2));
            }
        }

        __function void rv_and(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);
            reg_write(uintptr_t, rd, (v1 & v2));
        }

        __function void rv_remu(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, v1);
            reg_read(uintptr_t, rs2, v2);

            if (v2 == 0) {
                reg_write(uintptr_t, rd, 0);
            } else {
                reg_write(uintptr_t, rd, (v1 % v2));
            }
        }

        __function void rv_amoswap_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int64_t, address, v1);
                reg_write(int64_t, rd, v1);

                reg_read(int64_t, rs2, v2);
                mem_write(int64_t, address, v2);

                rvm64::memory::vm_clear_load_rsv();
            }
        }

        __function void rv_amoadd_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int64_t, address, v1);
                reg_read(int64_t, rs2, v2);

                mem_write(int64_t, address, (v1 + v2));
                reg_write(int64_t, rd, v1);

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoxor_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 ^ v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoand_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 & v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoor_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 | v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomin_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomax_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int64_t, rs2, v2);
                mem_read(int64_t, address, v1);

                reg_write(int64_t, rd, v1);
                mem_write(int64_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amominu_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomaxu_d(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint64_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint64_t, rs2, v2);
                mem_read(uint64_t, address, v1);

                reg_write(uint64_t, rd, v1);
                mem_write(uint64_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoswap_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                mem_read(int32_t, address, v1);
                reg_write(int32_t, rd, v1);

                reg_read(int32_t, rs2, v2);
                mem_write(int32_t, address, v2);

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoadd_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            mem_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 + v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoxor_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 ^ v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoand_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 & v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amoor_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);

            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 | v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomin_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomax_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            int32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(int32_t, rs2, v2);
                mem_read(int32_t, address, v1);

                reg_write(int32_t, rd, v1);
                mem_write(int32_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amominu_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint32_t, rs2, v2);
                mem_read(uint32_t, address, v1);

                reg_write(uint32_t, rd, v1);
                mem_write(uint32_t, address, (v1 < v2 ? v1 : v2));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }

        __function void rv_amomaxu_w(uint8_t rs2, uint8_t rs1, uint8_t rd) {
            uintptr_t address = 0;
            uint32_t v1 = 0, v2 = 0;

            reg_read(uintptr_t, rs1, address);
            while (true) {
                if (rvm64::memory::vm_check_load_rsv(address)) {
                    continue;
                }
                rvm64::memory::vm_set_load_rsv(address);

                reg_read(uint32_t, rs2, v2);
                mem_read(uint32_t, address, v1);

                reg_write(uint32_t, rd, v1);
                mem_write(uint32_t, address, (v1 < v2 ? v2 : v1));

                rvm64::memory::vm_clear_load_rsv();
                return;
            }
        }
    };
};
