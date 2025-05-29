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
	__vmcall void vm_entry() {
		while (!vmcs->halt) {
			rvm64::decoder::vm_decode(*(int32_t*)vmcs->pc);

			if (vmcs->csr.m_cause == environment_call_native) {
				vm_guard_ctx(rvni::vm_trap_exit());  
				continue;
			}
			if (vmcs->step) {
				vmcs->pc += 4; 
			}
		}
	}

	__function int64_t vm_main() {
		vmcs_t vm_instance = { };
		vmcs = &vm_instance;

		rvm64::context::vm_context_init();
		rvm64::rvni::resolve_ucrt_imports(); 

		while(!vmcs->halt) { 
			if (!mock::read_program_from_packet()) { 
				continue;
			}

			host_guard_ctx(rvm64::vm_entry());
			break;
		};

		return (int64_t)vmcs->reason;
	}
};

int main() {
    rvm64::vm_main();
}


