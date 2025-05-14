cmake_minimum_required(VERSION 3.31)
project(rvm64)

set(CMAKE_CXX_STANDARD 20)
add_executable(rvm64 vmcs.cpp vmcs.S
        vmcs.h
        vmctx.cpp
        vmctx.h
        vmmem.cpp
        vmmem.h
        vmcrypt.cpp
        vmcrypt.h)
