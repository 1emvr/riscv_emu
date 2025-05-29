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

	__vmcall void vm_entry(void) {
		while (!vmcs->halt) {
			int32_t opcode = *(int32_t*) vmcs->pc;

			rvm64::decoder::vm_decode(opcode);

			if (vmcs->csr.m_cause == environment_call_native) {
				rvm64::rvni::vm_trap_exit(); 
				continue;
			}
			if (vmcs->step) {
				vmcs->pc += 4; 
			}

			vmcs->step = true;
		}
	}

	__function int64_t vm_main(void) {
		vmcs_t vm_instance = { };
		vmcs = &vm_instance;

		rvm64::context::vm_context_init();
		rvm64::rvni::resolve_ucrt_imports(); 

		while(!vmcs->halt) { 
			if (!mock::read_program_from_packet()) { 
				continue;
			}

			// __vmcall
			rvm64::vm_entry();
			break;
		};

		return (int64_t)vmcs->reason;
	}
};

int main() {
    rvm64::vm_main();
}


