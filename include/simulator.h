#include <concepts>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace simulator
{

typedef char instrFlag;
typedef char reg;

enum struct instrType : char {
	UNK = 0, // Used for errors
	NOP, // Explicit NOP
	SNOP, // Soft NOP, used to fill pipeline stalls
	R3, // R-Type operation that takes 3 registers
	R2, // R-Type operation that takes 2 registers
	MEM, // Load and store operations
	BRA2, // Branch operations that compare 2 registers
	BRA1, // Branch operations that compare 1 register
	J // Inconditional jump
};

enum struct operation : char {
	NUL = 0, // Used for errors
	NONE, // For when operation is not relevant
	ADD,
	AND,
	NOR,
	OR,
	SUB,
	XOR,
	L, // Load
	S, // Store
	EQ, // Equal
	NE, // Not equal
	GEZ, // Greater or equal to zero
	GTZ, // Greater than zero
	LEZ, // Less or equal to zero
	LTZ // Less than zero
};

enum struct varType : char { VAR, ARRAY };

enum struct dataSize : char { WORD = 0, BYTE, HALF };

enum struct opMod : char { NONE = 0, IMMEDIATE, UNSIGNED, IMMEDIATE_UNSIGNED };

enum struct pipPhase : char { NONE = 0, FECTH = 1, DECODE = 2, EXECUTE = 3, MEMORY = 4, WRITEBACK = 5 };

enum struct regType : char { NONE = 0, RS, RT, RD };

enum struct forwardingType : char { NONE = 0, FULL, ALU };

enum struct branchPredType : char { NONE = 0, PERFECT, TAKEN, NOT_TAKEN };

class varDef
{
  public:
	varType type;
	dataSize size;
	char reg; // Register to save the address
	uint value;
};

class memory
{
	vector<char> internalMem;

  public:
	// Gets the value in memory at the given index or nullptr if out of bounds.
	template <integral T> T *get(uint idx)
	{
		if (idx < 0) return nullptr;

		uint end = idx + sizeof(T);
		if (internalMem.size() < end) return nullptr;

		return (T *)&internalMem[idx];
	}

	// Adds the variable to memory. Returns the resulting index in memory.
	int add(varDef def);

	// Frees unused memory. Should be called after all variables have been added.
	void shrink();

	inline int size()
	{
		return internalMem.size();
	}
};

class instruction
{
  public:
	string label;
	string displayName;
	instrType type;
	operation op;

	// Registers can be a number between 0 and 31 or -1 if unused.
	reg rS; // Operand 1
	reg rT; // Operand 2 or result
	reg rD; // Result

	union {
		dataSize size;
		opMod mod;
	} flags;

	short im;
	string labelOp;

	void execute(memory &mem, int regs[], unordered_map<string, int> &labelMap, int &pc);

	// Returns the first pipeline phase where the value is rS is needed.
	pipPhase calcRSNeeded();

	// Returns the first pipeline phase where the value is rT is needed.
	pipPhase calcRTNeeded();

	// Returns the first pipeline phase where the result is ready. (At the end of the phase)
	pipPhase calcResultDone();

	// Returns in which register the result is written.
	regType getRegWritten();

	string toString();
};
} // namespace simulator
