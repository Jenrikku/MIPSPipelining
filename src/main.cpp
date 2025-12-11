#include "translator.h"
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

int main(int argc, char *argv[])
{
	// Parse arguments
	ifstream iFile;
	ofstream oFile;
	bool useRegularNOPs = false;
	bool branchInDec = false;
	uint instrLimit = 256; // Instruction limit (to prevent infinite loops)
	simulator::forwardingType forwarding = simulator::forwardingType::NONE;
	simulator::branchPredType branchPred = simulator::branchPredType::NONE;

	int opt, optidx = 0;
	static struct option long_options[] = {{"input", required_argument, nullptr, 'i'},
										   {"output", required_argument, nullptr, 'o'},
										   {"nops", no_argument, nullptr, 'n'},
										   {"branch-in-dec", no_argument, nullptr, 'd'},
										   {"unlimited", no_argument, nullptr, 'u'},
										   {"forwarding", optional_argument, nullptr, 'f'},
										   {"branch", required_argument, nullptr, 'b'},
										   {"help", no_argument, nullptr, 'h'},
										   {nullptr, 0, nullptr, 0}};

	while ((opt = getopt_long(argc, argv, "hnudf::b:i:o:", long_options, &optidx)) != -1) {
		switch (opt) {
			case 'i':
				iFile = ifstream(optarg);
				if (!iFile.is_open()) {
					cerr << "Error: File " << optarg << " does not exist or cannot be opened." << endl;
					return -1;
				}

				break;

			case 'o':
				oFile = ofstream(optarg);
				if (!oFile.is_open()) {
					cerr << "Error: File " << optarg << " could not be opened for writing." << endl;
					return -1;
				}

				break;

			case 'n':
				useRegularNOPs = true;
				break;

			case 'd':
				branchInDec = true;
				break;

			case 'u':
				instrLimit = UINT32_MAX;
				break;

			case 'f': {
				string arg = optarg ? string(optarg) : "";
				if (arg.empty() || arg == "full") {
					forwarding = simulator::forwardingType::FULL;
				} else if (arg == "alu") {
					forwarding = simulator::forwardingType::ALU;
				} else if (arg != "no") {
					cerr << "Error: Unknown forwarding type " << arg << endl;
					return -1;
				}

				break;
			}

			case 'b': {
				string arg = string(optarg);
				if (arg == "p") {
					branchPred = simulator::branchPredType::PERFECT;
				} else if (arg == "t") {
					branchPred = simulator::branchPredType::TAKEN;
				} else if (arg == "nt") {
					branchPred = simulator::branchPredType::NOT_TAKEN;
				} else if (arg != "no") {
					cerr << "Error: Unknown branch prediction type " << arg << endl;
					return -1;
				}

				break;
			}

			case 'h':
				cout << "MIPS Pipeline Simulator Options\n"
						"\t-i --input\t\t\tSpecify the input file to read from.\n"
						"\t-o --output\t\t\tSpecify the ouput file to write to.\n"
						"\t-n --nops\t\t\tAdds NOPs to the resulting code rather than printing the time map.\n"
						"\t-d --branch-in-dec\t\tBranch jump address is calculated in the decode phase.\n"
						"\t-u --unlimited\t\t\tDisables hard limit on amount of executed instructions.\n"
						"\t-f --forwarding [no|alu|full]\tChoose between the following forwarding options:\n"
						"\t\t* no: No forwarding.\n\t\t* alu: Only ALU-ALU (EX to EX) forwarding.\n"
						"\t\t* full: Full forwarding.\n"
						"\t-b --branch [no|p|t|nt]\tChoose between the following branch prediction options:\n"
						"\t\t* no: No branch prediction.\n\t\t* p: Perfect branch prediction.\n"
						"\t\t* t: Always predict as taken.\n\t\t* nt: Always predict as not taken.\n"
						"\nNote that if no input/output file is specified then the standard input/output will be used."
					 << endl;

				return 0;

			case '?': // getopt_long handles error printing
				return -1;
		}
	}

	if (iFile.is_open()) cin.rdbuf(iFile.rdbuf());
	if (oFile.is_open()) cout.rdbuf(oFile.rdbuf());

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

	freeResources(); // Frees resources from the parser

	vector<simulator::instruction> executedCode;

	// For each register, stores how many pipeline stages are left before the register's value is available
	// (either by forwarding or write-back)
	char regBusy[32] = {};
	// For each register, stores how many pipeline stages are left before the register's value is written (write-back)
	char regDirty[32] = {};

	bool lastSNOP = false; // Used for ALU-ALU forwarding.

	// Execution
	for (uint pc = 0; pc < codeSize; pc++) {

		if (executedCode.size() > instrLimit) {
			cerr << "Instruction limit reached. Check for infinite loops." << endl;
			return -1;
		}

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

		simulator::instrType nopType = useRegularNOPs ? simulator::instrType::NOP : simulator::instrType::SNOP;
		simulator::instruction nop = {.displayName = "NOP", .type = nopType, .op = simulator::operation::NONE};

		if (addNOP) {
			executedCode.push_back(nop);
			lastSNOP = true;
			--pc;
			continue;
		}

		lastSNOP = false;

		uint lastpc = pc;

		instr.execute(dataMem, regs, labelMap, pc);
		executedCode.push_back(instr);

		if (instr.type == simulator::instrType::BRA1 || instr.type == simulator::instrType::BRA2) {
			bool taken = lastpc != pc;

			switch (branchPred) {
				case simulator::branchPredType::NONE:
					executedCode.push_back(nop);
					if (!branchInDec) executedCode.push_back(nop);
					break;

				case simulator::branchPredType::TAKEN:
					if (!taken) {
						executedCode.push_back(nop);
						if (!branchInDec) executedCode.push_back(nop);
					}
					break;

				case simulator::branchPredType::NOT_TAKEN:
					if (taken) {
						executedCode.push_back(nop);
						if (!branchInDec) executedCode.push_back(nop);
					}
					break;

				default:
					break;
			}

			continue;
		}

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

	uint pos = 0; // Position of the cursor in the time map.
	uint fetchpos = 0; // Position of the next fetch (first phase) in the time map.
	uint lastpos = 0; // Position of the last phase in the map (used for counting cycles).
	unordered_set<int> stalls; // Set of stalls already placed in the time map.

	int lasti = -1; // Stores last instruction that was not an SNOP.

	bool lastBranch = false; // Last instruction was a branch.

	auto printPhase = [&pos, &stalls](char phase) {
		while (stalls.contains(pos++)) {
			cout << "   ";
		}

		cout << phase << "  ";
	};

	for (uint i = 0; i < executedCode.size(); i++) {
		simulator::instruction instr = executedCode[i];
		string instrStr = instr.toString();
		if (instrStr.empty()) continue;

		cout << instrStr;
		if (useRegularNOPs) {
			cout << endl;
			continue;
		}

		cout << "\t\t";

		// Stalls before decoding phase.
		bool stallsDec = forwarding != simulator::forwardingType::FULL;

		for (int i = 0; i < pos; i++)
			cout << "   ";

		if (!lastBranch) printPhase('F');

		if (!stallsDec && !lastBranch) {
			printPhase('D');
			fetchpos = pos - 1;
		}

		if (lasti >= 0)
			for (int j = lasti + 1; executedCode[j].type == simulator::instrType::SNOP && j < executedCode.size();
				 j++) {

				cout << "S  ";
				stalls.insert(pos++);
			}

		if (lastBranch) printPhase('F');

		if (stallsDec || lastBranch) {
			printPhase('D');
			fetchpos = pos - 1;
		}

		cout << "X  M  W" << endl;
		lastpos = pos + 3;
		pos = fetchpos;
		lasti = i;
		lastBranch = instr.type == simulator::instrType::BRA1 || instr.type == simulator::instrType::BRA2;
	}

	if (!useRegularNOPs) cout << "\nCycles: " << lastpos << endl;
}