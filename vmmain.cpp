#include <windows.h>
#include <stdint.h>

#include "mock.hpp"
#include "vmmain.h"
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


