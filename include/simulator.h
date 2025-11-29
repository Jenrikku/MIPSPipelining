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

class instruction
{
  public:
	string label;
	string displayName;
	instrType type;
	operation op;

	reg rS; // Operand 1
	reg rT; // Operand 2 or result
	reg rD; // Result

	union {
		dataSize size;
		opMod mod;
	} flags;

	short im;
	string labelOp;
};

class varDef
{
  public:
	string label;
	varType type;
	dataSize size;
	uint value;
};

class memory
{
	vector<char> internalMem;
	unordered_map<string, uint> labelMap;

  public:
	// Gets the value in memory at the given index or nullptr if out of bounds.
	template <integral T> T *get(uint idx)
	{
		if (idx < 0) return nullptr;

		uint end = idx + sizeof(T);
		if (internalMem.size() < end) return nullptr;

		return &internalMem[idx];
	}

	// Gets the value in memory of the given label with an offset.
	// Returns nullptr if the label was not found or the offset is out of bounds.
	template <integral T> T *get(string label, int offset = 0)
	{
		if (!labelMap.contains(label)) return nullptr;

		uint idx = labelMap[label], start = idx + offset, end = start + sizeof(T);
		if (start < 0 || internalMem.size() < end) return nullptr;

		return &internalMem[start];
	}

	// Adds the variable to memory. Returns false if the label is already in use.
	bool add(varDef def);

	// Frees unused memory. Should be called after all variables have been added.
	void shrink();

	inline int size()
	{
		return internalMem.size();
	}
};
} // namespace simulator
