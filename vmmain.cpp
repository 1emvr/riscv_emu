#include <windows.h>
#include <stdint.h>

#include "mock.hpp"
#include "vmmain.hpp"
#include "vmctx.hpp"
#include "vmmem.hpp"
#include "vmcode.hpp"
#include "vmcrypt.hpp"
#include "rvni.hpp"

namespace rvm64 {

    _function void vm_entry(void) {
		rvm64::rvni::resolve_ucrt_imports();

        while (!vmcs->halt) {
            int32_t opcode = *(int32_t*) vmcs->pc;
            rvm64::decoder::vm_decode(opcode);

			if (!vmcs->step) {
				rvm64::rvni::vm_trap_exit();
				continue;
			}

			vmcs->pc += 4; // step while-not j/b-instruction
        }
    }

    _function int64_t vm_main(void) {
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


