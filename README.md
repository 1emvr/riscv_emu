# rvm64
RISC-V virtual machine

### Testing Phase
## Usage:
compile a risc-v binary object named "test.o" in the root folder and run.

Example:
```sh
clang --target=riscv64 -march=rv64g rotate.s -c -o rotate.o
./rvm64.exe
```

### Bugs:
- currently only supports math operations
- does not support a majority of floating-point operations
- does not support syscalls or any VM_EXIT
- no debug output
### TODO:
- implement rolling key decryption
- implement vm exit for syscalls
- implement full F/D Extension
