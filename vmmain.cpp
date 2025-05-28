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
		while (!vmcs->halt) {
			int32_t opcode = *(int32_t*) vmcs->pc;

			rvm64::decoder::vm_decode(opcode);

			if (!vmcs->step) {
				rvm64::rvni::vm_trap_exit(); // NOTE: VM_EXIT steps pc by 4
				continue;
			}

			vmcs->pc += 4; 
		}
	}

	_function int64_t vm_main(void) {
		vmcs_t vm_instance = { };
		vmcs = &vm_instance;

		rvm64::context::vm_context_init();
		rvm64::rvni::resolve_ucrt_imports(); // TODO: move outside of rvm64

		while(!vmcs->halt) { // NOTE: continue until program packet is read or halt
			if (!mock::read_program_from_packet()) { 
				continue;
			}

			rvm64::vm_entry();
			break;
		};

		return (int64_t)vmcs->reason;
	}
};

int main() {
    rvm64::vm_main();
}


