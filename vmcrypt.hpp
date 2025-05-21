#include "vmmain.hpp"
namespace rvm64::crypt {
    __inline __function uintptr_t decrypt_ptr(uintptr_t ptr) {
        return ptr ^ __key;
    }

    constexpr uintptr_t encrypt_ptr(uintptr_t ptr) {
        return ptr ^ __key;
    }
};
