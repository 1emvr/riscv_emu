#ifndef VMCRYPT_H
#define VMCRYPT_H
#include "vmmain.hpp"

namespace rvm64::crypt {
    // NOTE: xor for now. rolling decryption won't come for a while.
    constexpr uintptr_t encrypt_ptr(uintptr_t ptr) {
        return ptr ^ __key;
    }

    _function uintptr_t decrypt_ptr(uintptr_t ptr) {
        return ptr ^ __key;
    }
};
#endif // VMCRYPT_H
