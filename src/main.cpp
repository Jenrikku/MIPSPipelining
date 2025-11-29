#include "translator.h"
#include <iostream>
#include <vector>

int main()
{
	uint line = 0;

	simulator::memory mem;

	vector<instruction> code = parse(&cin);

	// Variable declarations
	for (; line < code.size(); line++) {
		instruction instr = code[line];
		simulator::varDef varDef;

		if (!translator::isVarDef(instr)) break;

		if (!translator::toVarDef(instr, varDef, cerr)) {
			cerr << "Error happened at instruction " << line + 1 << endl;
			return -1;
		}

		mem.add(varDef);
		cout << mem.size() << endl;
	}

	mem.shrink();

	// Instructions
	for (; line < code.size(); line++) {
		instruction instr = code[line];
		simulator::instruction instruction;

		if (translator::isVarDef(instr)) {
			cerr << "Error: Cannot define a variable in execution time. Instruction " << line + 1 << endl;
			return -1;
		}

		if (!translator::toInstruction(instr, instruction, cerr)) {
			cerr << "Error happened at instruction " << line + 1 << endl;
			return -1;
		}

		cout << instruction.label << "\tT: " << (int)instruction.type << " OP:" << (int)instruction.op
			 << " rS:" << (ushort)instruction.rS << " rT:" << (ushort)instruction.rT << " rD:" << (ushort)instruction.rD
			 << " F:" << (ushort)instruction.flags.mod << " IM:" << instruction.im << " LabelOP:" << instruction.labelOp
			 << endl;
	}

	freeResources(); // Free resources from the parser
}