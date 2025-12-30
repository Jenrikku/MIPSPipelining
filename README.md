# MIPS Pipeline Simulator

Making use of a reduced set of instructions from the MIPS architecture, this program can interpret and execute assembly code as well as find possible data hazards according to the standard 5-stage MIPS pipeline.  
  
This project was made as a task for the [UIB](https://uib.eu).

## Features

- Assembly code parsing using Flex and Bison.
- Memory allocation using simulator directives.
- Code execution with branching supported.
- Data hazard detection and correction.
- Generation of pipeline diagram.
- Optionally add NOP instructions to code (to fix data hazards).
- Forwarding support.
- Partial branch prediction support.

## Supported instructions

Here is a list of the supported instructions:

- No operation (`NOP`, `NOOP`)
- R-Type
    - `ADD`, `ADDI`, `ADDU`, `ADDIU`
    - `AND`, `ANDI`
    - `OR`, `ORI`
    - `NOR`, `NORI`
    - `XOR`, `XORI`
    - `SUB`, `SUBU`
- Memory
    - Load (`LB`, `LH`, `LW`)
    - Store (`SB`, `SH`, `SW`)
- Branches
    - Equal (`BEQ`)
    - Not equal (`BNE`)
    - ≥ 0 (`BGEZ`)
    - \> 0 (`BGTZ`)
    - ≤ 0 (`BLEZ`)
    - < 0 (`BLTZ`)
- Unconditional Jump (`J`)

## Building

### Linux

In order to build for Linux, make you sure have Flex, Bison, CMake, Make and a C++ compiler of your choice installed in your system. These tools tend to come in a bundle when installing developer tools in most distributions.

```
mkdir build
cd build
cmake ..
make
```

You may choose a different name for the build directory. The name is irrelevant.

### Windows

Building for Windows may be more challenging because of how this project uses Flex and Bison. You may attempt to build for Windows using a Linux machine:

- Install mingw-w64 cross-compiler tools.
- Find where the header files are installed in. For example: `/usr/x86_64-w64-mingw32/include/`
- Create a symlink in that folder pointing towards `/usr/include/FlexLexer.h`
- Execute cmake with the following arguments:  
`cmake .. -DCMAKE_CXX_COMPILER=/bin/x86_64-w64-mingw32-g++ -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++"`

Note that depending on the distribution, the path to the mingw compiler may be different.