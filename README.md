# rvm64
very simple virtual machine for emulating risc-v code in Windows environments.

### Testing Phase

## Requirements:
It's recommended to use linux/wsl environment for building rvm64. Currently not supporting Windows build environment.
- clang++/ lld
- mingw-w64
- musl (libs/headers used for risc-v binaries) - https://github.com/kraj/musl

## Installation:
Run the depencency script to install the necessary packages, build and install musl:
```
sudo ./install_dependencies.sh
```
Compile the vm context obj and the main vm:
```
x86_64-w64-mingw32-g++ vmctx.S -I. -c -o vmctx.o
x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ main.cpp vmctx.o -I. -o rvm64.exe
```
The musl includes/libs can be used to create a standalone risc-v binaries.
Allow the compiler to ignore unresolved symbols. rvm64 will patch the plt.

A simple test binary is included with the project:
```
clang++ test.cpp -ffreestanding -nostdlib --target=riscv64 -march=rv64g -I. -isystem /usr/local/musl/include -Wl,--unresolved-symbols=ignore-all -Wl,-e,main -Wl,-static -o test.elf
```
## Usage:

## TODO:
-build scripts
