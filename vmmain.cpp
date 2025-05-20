#include <windows.h>
#include <stdint.h>

#include "mock.hpp"
#include "vmmain.h"
#include "vmctx.h"
#include "vmmem.h"
#include "vmcode.h"
#include "vmcrypt.h"

namespace rvm64 {
    __function void vm_entry(void) {
        // TODO: create the main loop

        while (!vmcs->halt) {
            int32_t opcode = (int32_t) vmcs->pc;
            uintptr_t operation = rvm64::decoder::vm_decode(opcode);

            operation = rvm64::crypt::decrypt_ptr(operation);
            // select wrapper here

            if (vmcs->step) {
                vmcs->pc += 4; // step while-not j/b-instruction
            }
        }
    }

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


