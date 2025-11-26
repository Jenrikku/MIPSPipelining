#include "translator.h"

namespace translator
{
bool isVarDef(instruction &instr)
{
	string name = string(instr.name);
	return name.length() >= 3 && name.starts_with("DE");
}

bool toVarDef(instruction &instr, simulator::varDef &outRes, ostream err)
{
	if (instr.label == nullptr) {
		err << "Error: Variable definitions requires a label." << endl;
		return false;
	}

	if (instr.op.ptr == nullptr || instr.op.type != OPIM) {
		err << "Error: Variable definitions requires an immediate value." << endl;
		return false;
	}

	if (instr.rcount > 0 || instr.rlist != nullptr) {
		err << "Error: Variable definitions cannot have registers." << endl;
		return false;
	}

	string name = string(instr.name);
	int length = name.length();

	if (length < 3 || length > 4 || !name.starts_with("DE")) {
		err << "Error: Unknown instruction. (Variable definition?)" << endl;
		return false;
	}

	if (name[2] == 'F') {
		outRes.type = simulator::varType::VAR;
	} else if (name[2] == 'V') {
		outRes.type = simulator::varType::ARRAY;
	} else {
		err << "Error: Unknown variable definition directive." << endl;
		return false;
	}

	outRes.size = simulator::dataSize::WORD;

	if (name.length() > 3) {
		if (name[3] == 'B') {
			outRes.size = simulator::dataSize::BYTE;
		} else if (name[3] == 'H') {
			outRes.size = simulator::dataSize::HALF;
		} else {
			err << "Error: Unknown variable data type." << endl;
			return false;
		}
	}

	outRes.label = string(instr.label);
	outRes.value = *(int *)instr.op.ptr;
	return true;
}

bool toInstruction(instruction &instr, simulator::instruction &outRes, ostream err)
{
	string name = string(instr.name);
	int length = name.length(); // We assume length > 0 always thanks to parser code

	auto checkLabel = [&instr, &name, &err]() -> bool {
		if (instr.op.ptr == nullptr || instr.op.type != OPLABEL) {
			err << "Error: Instruction " << name << " requires a label as operand." << endl;
			return false;
		}

		return true;
	};

	auto checkIndirect = [&instr, &name, &err]() -> bool {
		if (instr.op.ptr == nullptr || instr.op.type != OPINDIRECT) {
			err << "Error: Instruction " << name << " requires an register indirect operand." << endl;
			return false;
		}

		return true;
	};

	auto checkImmediate = [&instr, &name, &err]() -> bool {
		if (instr.op.ptr == nullptr || instr.op.type != OPIM) {
			err << "Error: Instruction " << name << " requires an immediate operand." << endl;
			return false;
		}

		return true;
	};

	auto checkRegisters = [&instr, &name, &err](int rcount) -> bool {
		if (instr.rlist == nullptr || instr.rcount != rcount) {
			err << "Error: Instruction " << name << " requires a total of " << rcount << " registers but found "
				<< instr.rcount << '.' << endl;
			return false;
		}

		return true;
	};

	indirect indir;

	outRes.type = simulator::instrType::UNK;
	outRes.op = simulator::operation::NUL;

	switch (name[0]) {
		case 'S': // Store
			outRes.op = simulator::operation::S;
		case 'L': // Load
			if (length != 2) {
				err << "Error: Unknown instruction " << name << " (Memory instruction?)" << endl;
				return false;
			}

			if (!checkIndirect()) return false;
			if (!checkRegisters(1)) return false;

			outRes.type = simulator::instrType::MEM;
			if (outRes.type == simulator::instrType::UNK) outRes.op = simulator::operation::L;

			outRes.rS = instr.rlist[0];

			indir = *(indirect *)instr.op.ptr;
			outRes.im = indir.reg;
			outRes.rT = indir.reg;

			switch (name[1]) {
				case 'B':
					outRes.flags.size = simulator::dataSize::BYTE;
					break;

				case 'H':
					outRes.flags.size = simulator::dataSize::HALF;
					break;

				case 'W':
					outRes.flags.size = simulator::dataSize::WORD;
					break;

				default:
					err << "Error: Unknown instruction " << name << " (Memory instruction?)" << endl;
					return false;
			}

			return true;

		case 'B': // Branches
			// TO-DO
			break;

			// Others: R-Type
	}

	// TO-DO
}
} // namespace translator
