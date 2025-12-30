# MIPS Pipeline Simulator

Making use of a reduced set of instructions from the MIPS architecture, this program can interpret and execute assembly code as well as find possible data hazards according to the standard 5-stage MIPS pipeline.  
  
This project was made as a task for the [UIB](https://uib.eu).

## User Guide (English)

### Command line interface

The program as a whole is used through a command-line interface. The following set of options, all optional, can be given to the program to modify its behaviour:

- **-i --input**: Specifies the input file from which the assembly code will be read.
- **-o --output**: Specifies the output file where the program's results will be written.
- **-n --nops**: Instead of showing the pipeline diagram, add `NOP`s to the code so that there are no data hazards.
- **-d --branch-in-dec**: Simulates that the branch is calculated during the decode phase which means that the next instruction to execute is already known once the decode phase ends.
- **-u --unlimited**: In order to avoid infinite loops, there is a maximum number of instructions that may be executed in the simulator. This option nullifies the set limit.
- **-t --tabs**: Use tabs rather than spaces when printing the pipeline diagram phases.
- **-f --forwarding**: Allows specifying which of the following forwarding types to use:
    - **no**: No forwarding (default).
    - **alu**: Forwarding only in the execution phases.
    - **full**: All kinds of forwarding are available. If the option is specified without any value, then this one is used.
- **-b --branch**: Allows specifying which of the following kinds of branch prediction to use:
    - **no**: No branch prediction (default).
    - **p**: Perfect branch prediction. No stalls will ever happen in the pipeline due to branches.
    - **t**: Branches are always predicted as taken.
    - **nt**: Branches are always predicted as not taken.

The options follow the POSIX standard as well as the [GNU extensions](https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html).  
Note that if no input file is specified, then the terminal input will be used. The program will start the simulation once the end of the file is found, which can be sent in most terminals by pressing Ctrl+D.

### Assembly code

The assembly code can make use of both upper and lower case as the program is case-insensitive.  
When referring to registers, either the dollar sign (`$`) or the letter `R` can be used. So `$9` and `R9` are equivalent.  
To write comments, either `;`, `/`, `#` or `@` may be used. Any characters found after these symbols will be ignored until a line break.  
  
Each line may optionally start with a label in the following way:

```
LABEL: XORI $4, $3, 4
```

These labels will be used for branch and jump instructions.

#### Supported instructions

The assembly code may contain the following instructions:

| Instruction    | Use               | Description                                                                                   |
| -------------- | ----------------- | --------------------------------------------------------------------------------------------- |
| `NOP` / `NOOP` | `NOP`             | No operation.                                                                                 |
| `ADD`          | `ADD $d, $s, $t`  | Adds the values of `$s` and `$t` and puts the result in `$d`.                                 |
| `ADDI`         | `ADDI $t, $s, i`  | Adds the immediate value `i` and `$s` and puts the result in `$t`.                            |
| `ADDU`         | `ADDU $d, $s, $t` | Does the same as `ADD` but for unsigned values.                                               |
| `ADDIU`        | `ADDIU $t, $s, i` | Does the same as `ADDI` but for unsigned values.                                              |
| `AND`          | `AND $d, $s, $t`  | Does a bitwise and of the values in `$s` and `$t` and puts the result in `$d`.                |
| `ANDI`         | `ANDI $d, $s, i`  | Does a bitwise and of `$s` and the immediate value `i` and puts the result in `$t`.           |
| `OR`           | `OR $d, $s, $t`   | Does a bitwise or of the values in `$s` and `$t` and puts the result in `$d`.                 |
| `ORI`          | `ORI $d, $s, i`   | Does a bitwise or of `$s` and the immediate value `i` and puts the result in `$t`.            |
| `NOR`          | `NOR $d, $s, $t`  | Does a bitwise nor of the values in `$s` and `$t` and puts the result in `$d`.                |
| `NORI`         | `NORI $d, $s, i`  | Does a bitwise nor of `$s` and the immediate value `i` and puts the result in `$t`.           |
| `XOR`          | `XOR $d, $s, $t`  | Does a bitwise xor of the values in `$s` and `$t` and puts the result in `$d`.                |
| `XORI`         | `XORI $d, $s, i`  | Does a bitwise xor of `$s` and the immediate value `i` and puts the result in `$t`.           |
| `SUB`          | `SUB $d, $s, $t`  | Subtracts the value of `$t` to the value of `$s` (s - t) and puts the result in `$d`.         |
| `SUBU`         | `SUBU $d, $s, $t` | Does the same as `SUB` but for unsigned values.                                               |
| `LB`           | `LB $t, i($s)`    | Loads a byte (8 bits) to `$t` from the memory address pointed to by `$s + i`.                 |
| `LH`           | `LH $t, i($s)`    | Loads a half word (16 bits) to `$t` from the memory address pointed to by `$s + i`.           |
| `LW`           | `LW $t, i($s)`    | Loads a word (32 bits) to `$t` from the memory address pointed to by `$s + i`.                |
| `SB`           | `SB $t, i($s)`    | Stores a byte (8 bits) from `$t` to the memory address pointed to by `$s + i`.                |
| `SH`           | `SH $t, i($s)`    | Stores a half word (16 bits) from `$t` to the memory address pointed to by `$s + i`.          |
| `SW`           | `SW $t, i($s)`    | Stores a word (32 bits) from `$t` to the memory address pointed to by `$s + i`.               |
| `BEQ`          | `BEQ $s, $t, lab` | If the values in `$s` and `$t` are equal, jumps to the instruction with the label `lab`.      |
| `BNE`          | `BNE $s, $t, lab` | If the values in `$s` and `$t` are not equal, jumps to the instruction with the label `lab`.  |
| `BGEZ`         | `BGEZ $s, lab`    | If the value in `$s` is greater or equal to 0, jumps to the instruction with the label `lab`. |
| `BGTZ`         | `BGTZ $s, lab`    | If the value in `$s` is greater than 0, jumps to the instruction with the label `lab`.        |
| `BLEZ`         | `BLEZ $s, lab`    | If the value in `$s` is less or equal to 0, jumps to the instruction with the label `lab`.    |
| `BLTZ`         | `BLTZ $s, lab`    | If the value in `$s` is less than 0, jumps to the instruction with the label `lab`.           |
| `J`            | `J lab`           | Always jumps to the instruction with the label `lab`.                                         |

#### Memory allocation

In order to allocate memory, the following simulator directives can be used, always **before the code** that will execute.

| Directive      | Use               | Description                                                                                            |
| -------------- | ----------------- | ------------------------------------------------------------------------------------------------------ |
| `DEFB`         | `DEFB $s, i`      | Adds a 1-byte (8 bits) variable with value `i` and puts its address into the register `$s`.            |
| `DEFH`         | `DEFH $s, i`      | Adds a 2-byte (16 bits) variable with value `i` and puts its address into the register `$s`.           |
| `DEFW`         | `DEFW $s, i`      | Adds a 4-byte (32 bits) variable with value `i` and puts its address into the register `$s`.           |
| `DEVB`         | `DEVB $s, i`      | Allocates an array of `i` values of 1 byte (8 bits) each and puts the first value's address in `$s`.   |
| `DEVH`         | `DEVH $s, i`      | Allocates an array of `i` values of 2 bytes (16 bits) each and puts the first value's address in `$s`. |
| `DEVW`         | `DEVW $s, i`      | Allocates an array of `i` values of 4 bytes (32 bits) each and puts the first value's address in `$s`. |

All arrays are initialized to 0.

### Considerations

It has been considered that unconditional jump instructions (`J`) will always introduce a pipeline stall since we do not know that it is a jump until the decode phase.  
By default and if the option `branch-in-dec` is not specified, for all branch instructions we do not know if they jump until the execution phase.

