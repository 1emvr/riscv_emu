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
    };
};
