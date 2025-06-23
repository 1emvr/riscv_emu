cmake_minimum_required(VERSION 3.31)
project(rvm64)

set(CMAKE_CXX_STANDARD 20)
add_executable(rvm64
        rvni.hpp
        vmcode.hpp
        vmcommon.hpp
        vmcrypt.hpp
        mock.hpp
        main.cpp
        vmelf.hpp
        vmmain.hpp
        vmmem.hpp
        vmrwx.hpp
        vmveh.hpp
        vmctx.S
        vmutils.hpp)
