#ifndef VMCODE_H
#define VMCODE_H
#include "vmmain.h"

namespace rvm64::decoder {
    struct opcode {
        uint8_t mask;
        typenum type;
    };
    __rdata const opcode encoding[] = {
        { 0b1010011, rtype  }, { 0b1000011, rtype  }, { 0b0110011, rtype  }, { 0b1000111, r4type }, { 0b1001011, r4type }, { 0b1001111, r4type },
        { 0b0000011, itype  }, { 0b0001111, itype  }, { 0b1100111, itype  }, { 0b0010011, itype  }, { 0b1110011, itype  }, { 0b0100011, stype  },
        { 0b0100111, stype  }, { 0b1100011, btype  }, { 0b0010111, utype  }, { 0b0110111, utype  }, { 0b1101111, jtype  },
    };

    __function uintptr_t vm_decode(uint32_t opcode);
};
#endif //VMCODE_H
