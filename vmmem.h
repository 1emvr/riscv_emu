#ifndef VMMEM_H
#define VMMEM_H
#include "mono.hpp"

namespace rvm64::memory {
    __function void vm_init(void);
    __function void vm_end(void);
    __function void vm_clear_load_rsv(void);
    __function void vm_set_load_rsv(uintptr_t address);
    __function bool vm_check_load_rsv(uintptr_t address);
    __function void vm_stkchk(uintptr_t sp);
};
#endif //VMMEM_H
