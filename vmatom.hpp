#ifndef VMATOM_H
#define VMATOM_H
#include "vmmain.hpp"

namespace rvm64::atom {
    __function void vm_set_load_rsv(int hart_id, uintptr_t address) {
        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = address; // vmcs_array[hart_id]->load_rsv_addr = address;
        vmcs->load_rsv_valid = true; // vmcs_array[hart_id]->load_rsv_valid = true;

        ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    __function void vm_clear_load_rsv(int hart_id) {
        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);

        vmcs->load_rsv_addr = 0LL; // vmcs_array[hart_id]->load_rsv_addr = 0LL;
        vmcs->load_rsv_valid = false; // vmcs_array[hart_id]->load_rsv_valid = false;

        ctx->win32.NtReleaseMutex(vmcs_mutex);
    }

    __function bool vm_check_load_rsv(int hart_id, uintptr_t address) {
        int valid = 0;

        ctx->win32.NtWaitForSingleObject(vmcs_mutex, INFINITE);
        valid = (vmcs->load_rsv_valid && vmcs->load_rsv_addr == address); // (vmcs_array[hart_id]->load_rsv_valid && vmcs_array[hart_id]->load_rsv_addr == address)

        ctx->win32.NtReleaseMutex(vmcs_mutex);
        return valid;
    }
};
#endif //VMATOM_H
