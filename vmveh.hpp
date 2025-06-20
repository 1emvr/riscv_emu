#ifndef VMVEH_H
#define VMVEH_H
#include "vmmain.hpp"
#include "rvni.hpp"

LONG CALLBACK vm_exception_handler(PEXCEPTION_POINTERS exception_info) {
	DWORD code = exception_info->ExceptionRecord->ExceptionCode;

	if (code == STATUS_SINGLE_STEP) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	CSR_GET(exception_info);

	if (vmcs->halt || code != EXCEPTION_BREAKPOINT) {
		exception_info->ContextRecord->Rip = (DWORD64)vmcs->trap_handler;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	if (vmcs->csr.m_cause == environment_call_native) {
		rvm64::rvni::vm_native_call();
		vmcs->halt = 0;
	}

	return EXCEPTION_CONTINUE_EXECUTION;
}

#endif //VMVEH_H
