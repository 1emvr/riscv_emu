#include <windows.h>
#include <stdint.h>

#include "mock.hpp"
#include "vmmain.hpp"
#include "vmctx.hpp"
#include "vmmem.hpp"
#include "vmcode.hpp"
#include "vmcrypt.hpp"

namespace rvm64 {
    __function void vm_entry(void) {
        while (!vmcs->halt) {

            int32_t opcode = *(int32_t*) vmcs->pc;
            rvm64::decoder::vm_decode(opcode);

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

        rvm64::vm_entry();
        rvm64::memory::vm_end();

        return vmcs->reason;
    }
};

int main() {
    rvm64::vm_main();
}


