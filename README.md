# rvm64
very simple virtual machine for emulating risc-v code in Windows environments.

### Testing Phase

## Requirements:
It's recommended to use linux/wsl environment for building rvm64. Currently not supporting Windows build environment.
- clang++/ lld
- mingw-w64
- musl (libs/headers used for risc-v binaries) - https://github.com/kraj/musl

## Installation:
run the depencency script to install the necessary packages, build and install musl:
```sh
# ./install_dependencies.sh
```
compile the vm context obj and the main vm:
```sh
$ x86_64-w64-mingw32-g++ vmctx.S -I. -c -o vmctx.o
$ x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ main.cpp vmctx.o -I. -o rvm64.exe
```
## Usage:

## TODO:
-build scripts
