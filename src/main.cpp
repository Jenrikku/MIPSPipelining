#include "translator.h"
#include <iostream>
#include <vector>

int main()
{
	uint line = 0;

	simulator::memory dataMem;
	int regs[32] = {};

	vector<instruction> instrs = parse(&cin);

	// Variable declarations
	for (; line < instrs.size(); line++) {
		instruction instr = instrs[line];
		simulator::varDef varDef;

		if (!translator::isVarDef(instr)) break;

		if (!translator::toVarDef(instr, varDef, cerr)) {
			cerr << "Error happened at instruction " << line + 1 << endl;
			return -1;
		}

		regs[varDef.reg] = dataMem.add(varDef);
	}

	dataMem.shrink();

	simulator::instruction code[instrs.size() - line];

	// Instructions
	for (int i = 0; line < instrs.size(); line++, i++) {
		instruction instr = instrs[line];
		simulator::instruction instruction;

		if (translator::isVarDef(instr)) {
			cerr << "Error: Cannot define a variable in execution time. Instruction " << line + 1 << endl;
			return -1;
		}

		if (!translator::toInstruction(instr, instruction, cerr)) {
			cerr << "Error happened at instruction " << line + 1 << endl;
			return -1;
		}

		code[i] = instruction;
	}

	for (simulator::instruction instr : code) {
		cout << instr.label << "\t" << instr.displayName << "\tT: " << (int)instr.type << " OP: " << (int)instr.op
			 << " rS: " << (ushort)instr.rS << " rT: " << (ushort)instr.rT << " rD: " << (ushort)instr.rD
			 << " F: " << (ushort)instr.flags.mod << " IM: " << instr.im << " LabelOP: " << instr.labelOp << endl;
	}

	freeResources(); // Frees resources from the parser
}