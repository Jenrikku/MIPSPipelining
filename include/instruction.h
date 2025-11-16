#include <cstdint>
#include <string>

using namespace std;

namespace pipeline
{
// // --- Custom ---
// DEF,
// DEFB,
// DEFH,
// DEFW,
// DEV,
// DEVB,
// DEVH,
// DEVW,
// // --- Standard MIPS ---
// NOP,
// // 3-operand R-type instructions
// ADD,
// ADDU,
// ADDI,
// ADDIU,
// AND,
// ANDI,
// NOR,
// OR,
// ORI,
// SLL,
// SLLV,
// SRA,
// SRAV,
// SRL,
// SRLV,
// SUB,
// SUBU,
// XOR,
// XORI,
// // 2-operand R-type instructions
// DIV,
// DIVU,
// MULT,
// MULTU,
// // Set comparisons
// SLT,
// SLTU,
// SLTI,
// SLTIU,
// // Branches
// BEQ,
// BGTZ,
// BLEZ,
// BNE,
// // Jumps
// J,
// JR,
// // Loads and stores
// LB,
// LBU,
// LH,
// LHU,
// LW,
// SB,
// SH,
// SW

class instruction
{
  public:
	string op;
	uint16_t r0;
	uint16_t r1;
	uint16_t r2;

	string label; // Label the instruction is associated to.
	string labelOp; // Label used for memory or jump operations.
};

} // namespace pipeline
