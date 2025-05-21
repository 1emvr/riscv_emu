cmake_minimum_required(VERSION 3.31)
project(rvm64)

set(CMAKE_CXX_STANDARD 20)
add_executable(rvm64 vmmain.cpp vmcs.S
        vmmain.hpp
        vmctx.hpp
        vmmem.hpp
        vmcrypt.hpp
        vmops.hpp
        vmcode.hpp
        vmelf.hpp)
