Bare metal experiments

You must use GCC cross (as described [this](https://wiki.osdev.org/GCC_Cross-Compiler) article on osdev wiki) compiller and CMake to build sources.
"boot.bin" will be loaded at 0x10000 linear addres with 0x1000:0 real-mode entry point.
