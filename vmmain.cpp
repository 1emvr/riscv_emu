#include <windows.h>
#include <stdint.h>

#include "mock.hpp"
#include "vmmain.hpp"
#include "vmctx.hpp"
#include "vmmem.hpp"
#include "vmcode.hpp"
#include "vmcrypt.hpp"

namespace rvm64 {
	__function bool vm_trap_host_call() {
		uintptr_t start = vmcs->process.address; 
		uintptr_t end = start + vmcs->process.size;

		if ((vmcs->pc < start) || (vmcs->pc >= end)) {
			auto it = thunk_table.find((void*)vmcs->pc);

			if (it != thunk_table.end()) {
				rvm64::context::save_vm_context();

				thunk *t = it->second;
				uint64_t result = t->wrapper(t->target, &vmcs->vregs);

				rvm64::context::restore_vm_context();

				reg_write(uint64_t, regenum::a0, result);
				vmcs->pc = vmcs->vregs[regenum::ra];

				return true;
			}				
		}
		return false;
	}

    __function void vm_entry(void) {
        while (!vmcs->halt) {

            int32_t opcode = *(int32_t*) vmcs->pc;
            rvm64::decoder::vm_decode(opcode);

			if (!vmcs->step) {
				if (!vm_trap_host_call()) {
					vmcs->halt = 1;
					vmcs->reason = vm_invalid_pc;
					continue;
				}
			}

			vmcs->pc += 4; // step while-not j/b-instruction
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


