# rvm64
very simple virtual machine for emulating risc-v code in Windows environments.

### Testing Phase

## Requirements:
It's recommended to use linux/wsl environment for building rvm64. Currently not scripting for any Windows build environment.
- clang++/ lld
- mingw-w64
- musl (libs/headers used for risc-v binaries) - https://github.com/kraj/musl
- riscv-gnu-toolchain (optional - eg: objdump, readelf) - https://github.com/riscv-collab/riscv-gnu-toolchain

## Installation:
Run the depencency script to install the necessary packages, build and install musl:
```
# ./install_dependencies.sh
```
Compile the vm context obj and the main vm:
```sh
$ x86_64-w64-mingw32-g++ vmctx.S -I. -c -o vmctx.o
$ x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ main.cpp vmctx.o -I. -o rvm64.exe
```
The musl includes/libs can be used to create a standalone risc-v binaries.
A simple test binary is included with the project:
```sh
$ clang++ test.cpp -ffreestanding -nostdlib --target=riscv64 -march=rv64g -I. -isystem /usr/local/musl/include -Wl,-shared -Wl,-e,main -o test.elf
```
Currently, the vm is hard-coded to read the file `test.elf`. A feature to specify files will come later. I'm doing this on my free time.

### NOTE:
When writing C++ code, the main function within the risc-v ELF should be compiled using C linkage until I find a better way:
```cpp
#include <string.h>
#include <stdlib.h>

// NOTE: do not mangle "main"
extern "C" int main() {
	...
}
```

## TODO:
- build scripts for other platforms
- adding user binary input
- actually testing the damn thing...
