#include "translator.h"
#include <iostream>
#include <unordered_map>
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

	int codeSize = instrs.size() - line;
	simulator::instruction code[codeSize];
	unordered_map<string, int> labelMap;

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

		if (!instruction.label.empty()) {
			if (labelMap.contains(instruction.label)) {
				cerr << "Error: cannot have the same label for more than one instruction. Instruction " << line + 1
					 << " for label " << instruction.label << endl;
				return -1;
			}

			labelMap[instruction.label] = i;
		}

		if (!instruction.labelOp.empty()) {
			if (!labelMap.contains(instruction.labelOp)) {
				cerr << "Error: reference to unknown label " << instruction.labelOp << " in instruction " << line + 1
					 << endl;
				return -1;
			}
		}

		code[i] = instruction;
	}

	// Execution
	for (int pc = 0; pc < codeSize; pc++) {
		simulator::instruction instr = code[pc];

		instr.execute(dataMem, regs, labelMap, pc);

		string instrStr = instr.toString();
		if (!instrStr.empty()) {
			cout << instrStr << endl;
		}
	}

	freeResources(); // Frees resources from the parser
}