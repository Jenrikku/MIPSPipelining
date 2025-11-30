#include "simulator.h"
#include <string>

namespace simulator
{
int memory::add(varDef def)
{
	char dataSize;
	switch (def.size) {
		case simulator::dataSize::HALF:
			dataSize = 2;
			break;
		case simulator::dataSize::BYTE:
			dataSize = 1;
			break;
		default:
			dataSize = 4;
	}

	int mod = internalMem.size() % dataSize;
	if (mod) { // Data alignment
		internalMem.resize(internalMem.size() + dataSize - mod, 0);
	}

	int index = internalMem.size();

	if (def.type == simulator::varType::ARRAY) {
		uint n = dataSize * def.value;
		internalMem.resize(internalMem.size() + n, 0);
		return index;
	}

	for (char i = 0; i < dataSize; i++) {
		char v = def.value >> i * 8;
		internalMem.push_back(v);
	}

	return index;
}

void memory::shrink()
{
	internalMem.shrink_to_fit();
}

void instruction::execute(memory &mem, int regs[], unordered_map<string, int> &labelMap, int &pc)
{
	if (type == instrType::UNK || type == instrType::NOP || type == instrType::SNOP || op == operation::NUL) return;

	int *resptr;
	int op1, op2;

	switch (type) {
		case instrType::J:
			pc = labelMap[labelOp] - 1; // - 1 because it increments after every instruction
			return;

		case instrType::R3:
			resptr = &regs[rD];
			op1 = regs[rS];
			op2 = regs[rT];
			break;

		case instrType::R2:
		case instrType::MEM:
			// Note: In MIPS, even unsigned operations perform a signed extension on immediate value.
			resptr = &regs[rT];
			op1 = regs[rS];
			op2 = im; // Implicit sign-extension.
			break;

		case instrType::BRA2:
			op2 = regs[rT];
		case instrType::BRA1:
			op1 = regs[rS];
			break;

		default: // Do nothing
			break;
	}

	bool memWrite = false;
	bool jump = false;

	switch (op) {
		case operation::ADD:
			if ((char)flags.mod & (char)opMod::UNSIGNED) {
				*resptr = (uint)op1 + (uint)op2;
				break;
			}

			*resptr = op1 + op2;
			break;

		case operation::AND:
			*resptr = op1 & op2;
			break;

		case operation::NOR:
			*resptr = ~(op1 | op2);
			break;

		case operation::OR:
			*resptr = op1 | op2;
			break;

		case operation::SUB:
			if ((char)flags.mod & (char)opMod::UNSIGNED) {
				*resptr = (uint)op1 - (uint)op2;
				break;
			}

			*resptr = op1 - op2;
			break;

		case operation::XOR:
			*resptr = op1 ^ op2;
			break;

		case operation::S:
			memWrite = true;
		case operation::L:
			switch (flags.size) {

				case dataSize::WORD: {
					int *memptr = mem.get<int>(op1 + op2);
					if (memptr == nullptr) return;

					if (memWrite)
						*memptr = *resptr;
					else
						*resptr = *memptr;

					break;
				}

				case dataSize::HALF: {
					short *memptr = mem.get<short>(op1 + op2);
					if (memptr == nullptr) return;

					if (memWrite)
						*memptr = *resptr;
					else
						*resptr = *memptr;

					break;
				}

				case dataSize::BYTE: {
					char *memptr = mem.get<char>(op1 + op2);
					if (memptr == nullptr) return;

					if (memWrite)
						*memptr = *resptr;
					else
						*resptr = *memptr;

					break;
				}
			}

			break;

		case operation::EQ:
			jump = op1 == op2;
			break;

		case operation::NE:
			jump = op1 != op2;
			break;

		case operation::GEZ:
			jump = op1 >= 0;
			break;

		case operation::GTZ:
			jump = op1 > 0;
			break;

		case operation::LEZ:
			jump = op1 <= 0;
			break;

		case operation::LTZ:
			jump = op1 < 0;
			break;

		default: // Do nothing
			break;
	}

	if (jump) pc = labelMap[labelOp] - 1; // - 1 because it increments after every instruction
}

string instruction::toString()
{
	if (type == instrType::UNK || type == instrType::SNOP) return "";

	string res = (label.empty() ? "\t" : label + ":\t") + displayName + ' ';

	switch (type) {
		case instrType::R3:
			res += '$' + to_string(rD) + ", $" + to_string(rS) + ", $" + to_string(rT);
			break;

		case instrType::R2:
			res += '$' + to_string(rT) + ", $" + to_string(rS) + ", " + to_string(im);
			break;

		case instrType::MEM:
			res += '$' + to_string(rT) + ", " + to_string(im) + "($" + to_string(rS) + ')';
			break;

		case instrType::BRA2:
			res += '$' + to_string(rS) + ", $" + to_string(rT) + ", " += labelOp;
			break;

		case instrType::BRA1:
			res += '$' + to_string(rS) + ", ";
		case instrType::J:
			res += labelOp;
			break;

		default:
			break;
	}

	return res;
}

} // namespace simulator
