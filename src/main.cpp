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
	simulator::forwardingType forwarding = simulator::forwardingType::ALU;
	// TO-DO: Read forwarding type from argument flag.

	bool useRegularNOPs = true;
	// TO-DO: Read NOP type from argument flag.

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

	vector<simulator::instruction> executedCode;

	// For each register, stores how many pipeline stages are left before the register's value is available
	// (either by forwarding or write-back)
	char regBusy[32] = {};
	// For each register, stores how many pipeline stages are left before the register's value is written (write-back)
	char regDirty[32] = {};

	bool lastSNOP = false; // Used for ALU-ALU forwarding.

	// Execution
	for (int pc = 0; pc < codeSize; pc++) {

		// TO-DO: Add hard limit on number of instructions
		//        Maybe add a flag to disable it.

		for (int i = 0; i < 32; i++) {
			if (regBusy[i] > 0) --regBusy[i];
			if (regDirty[i] > 0) --regDirty[i];
		}

		simulator::instruction instr = code[pc];

		simulator::pipPhase rSPhase = instr.calcRSNeeded();
		simulator::pipPhase rTPhase = instr.calcRTNeeded();

		bool addNOP = false;

		switch (forwarding) {
			case simulator::forwardingType::FULL:
				if ((char)rSPhase > 0 && instr.rS > 0) addNOP |= regBusy[instr.rS] >= (char)rSPhase;
				if ((char)rTPhase > 0 && instr.rT > 0) addNOP |= regBusy[instr.rT] >= (char)rTPhase;
				break;

			case simulator::forwardingType::ALU:
				if ((char)rSPhase > 0 && instr.rS > 0)
					addNOP |= regBusy[instr.rS] > 2 || lastSNOP && regDirty[instr.rS] > 0;

				if ((char)rTPhase > 0 && instr.rT > 0)
					addNOP |= regBusy[instr.rT] > 2 || lastSNOP && regDirty[instr.rT] > 0;

				break;

			case simulator::forwardingType::NONE:
				if ((char)rSPhase > 0 && instr.rS > 0) addNOP |= regDirty[instr.rS] > 0;
				if ((char)rTPhase > 0 && instr.rT > 0) addNOP |= regDirty[instr.rT] > 0;
				break;
		}

		if (addNOP) {
			// cout << (int)rSPhase << ' ' << (int)regBusy[instr.rS] << ' ' << (int)rTPhase << ' '
			// 	 << (int)regBusy[instr.rT] << endl;

			simulator::instrType nopType = useRegularNOPs ? simulator::instrType::NOP : simulator::instrType::SNOP;
			executedCode.push_back({.displayName = "NOP", .type = nopType, .op = simulator::operation::NONE});

			lastSNOP = true;
			--pc;
			continue;
		}

		lastSNOP = false;

		instr.execute(dataMem, regs, labelMap, pc);
		executedCode.push_back(instr);

		char regWrittenIdx = -1;

		switch (instr.getRegWritten()) {
			case simulator::regType::RS:
				regWrittenIdx = instr.rS;
				break;
			case simulator::regType::RT:
				regWrittenIdx = instr.rT;
				break;
			case simulator::regType::RD:
				regWrittenIdx = instr.rD;
				break;
			default:
				break;
		}

		if (regWrittenIdx < 0) continue;

		regBusy[regWrittenIdx] = (char)instr.calcResultDone();
		regDirty[regWrittenIdx] = (char)simulator::pipPhase::WRITEBACK - 2; // minus F & WB
	}

	for (simulator::instruction instr : executedCode) {
		string instrStr = instr.toString();
		if (!instrStr.empty()) cout << instrStr << endl;
	}

	freeResources(); // Frees resources from the parser
}