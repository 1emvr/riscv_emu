#include "vmmain.hpp"
namespace rvm64::crypt {
    // NOTE: xor for now. rolling decryption won't come for a while.
    __inline __function uintptr_t decrypt_ptr(uintptr_t ptr) {
        return ptr ^ __key;
    }

    constexpr uintptr_t encrypt_ptr(uintptr_t ptr) {
        return ptr ^ __key;
    }
};
