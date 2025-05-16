#ifndef VMCODE_H
#define VMCODE_H
#include "vmmain.h"

namespace rvm64::decoder {
    __function uintptr_t vm_decode(uint32_t opcode);
};
#endif //VMCODE_H
