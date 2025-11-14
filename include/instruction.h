namespace pipeline
{
enum opcode {
	// --- Custom ---
	DEF,
	DEV,
	// --- Standard MIPS ---
	NOP,
	// 3-operand R-type instructions
	ADD,
	ADDU,
	ADDI,
	ADDIU,
	AND,
	ANDI,
	NOR,
	OR,
	ORI,
	SLL,
	SLLV,
	SRA,
	SRAV,
	SRL,
	SRLV,
	SUB,
	SUBU,
	XOR,
	XORI,
	// 2-operand R-type instructions
	DIV,
	DIVU,
	MULT,
	MULTU,
	// Set comparisons
	SLT,
	SLTU,
	SLTI,
	SLTIU,
	// Branches
	BEQ,
	BGTZ,
	BLEZ,
	BNE,
	// Jumps
	J,
	JR,
	// Loads and stores
	LB,
	LBU,
	LH,
	LHU,
	LW,
	SB,
	SH,
	SW
};

struct instruction {
};
} // namespace pipeline
