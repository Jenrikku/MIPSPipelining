#include "translator.h"
#include <string>

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

	switch (name[2]) {
		case 'F':
			outRes.type = simulator::varType::VAR;
			break;

		case 'V':
			outRes.type = simulator::varType::ARRAY;
			break;

		default:
			err << "Error: Unknown variable definition directive." << endl;
			return false;
	}

	outRes.size = simulator::dataSize::WORD;

	if (name.length() > 3) {
		switch (name[3]) {
			case 'B':
				outRes.size = simulator::dataSize::BYTE;
				break;

			case 'H':
				outRes.size = simulator::dataSize::HALF;
				break;

			case 'W': // Do nothing
				break;

			default:
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
			err << "Error: Instruction " << name << " requires a register indirect operand." << endl;
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

	auto checkNoOP = [&instr, &name, &err]() -> bool {
		if (instr.op.type != OPNONE) {
			err << "Error: Instruction " << name << " has too many operands." << endl;
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

	auto errorUnkInstr = [&name, &err](string type) {
		err << "Error: Unknown instruction " << name << " (" << type << " instruction?)" << endl;
	};

	auto errorRZero = [&name, &err]() {
		err << "Error: Instruction " << name << " tried to write to constant value $0." << endl;
	};

	indirect indir;

	if (instr.label != nullptr) outRes.label = string(instr.label);
	outRes.displayName = string(instr.name);

	outRes.type = simulator::instrType::UNK;
	outRes.op = simulator::operation::NUL;

	switch (name[0]) {
		case 'S': // Store
			outRes.op = simulator::operation::S;
		case 'L': // Load
			if (length != 2) {
				errorUnkInstr("Memory");
				return false;
			}

			if (!checkIndirect()) return false;
			if (!checkRegisters(1)) return false;

			outRes.type = simulator::instrType::MEM;

			outRes.rT = instr.rlist[0];

			if (outRes.type == simulator::instrType::UNK) { // Not an store, so it's a load.
				outRes.op = simulator::operation::L;

				if (outRes.rT == 0) {
					errorRZero();
					return false;
				}
			}

			indir = *(indirect *)instr.op.ptr;
			outRes.im = indir.reg;
			outRes.rS = indir.reg;

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
					errorUnkInstr("Memory");
					return false;
			}

			return true;

		case 'B': // Branches
			if (length != 3 || length != 4) {
				errorUnkInstr("Branch");
				return false;
			}

			if (!checkLabel()) return false;
			outRes.labelOp = string((char *)instr.op.ptr);

			if (length == 3) { // Branch with 2 registers
				outRes.type = simulator::instrType::BRA2;
				if (!checkRegisters(2)) return false;

				outRes.rS = instr.rlist[0];
				outRes.rT = instr.rlist[1];

				if (name == "BEQ") {
					outRes.op = simulator::operation::EQ;
				} else if (name == "BNE") {
					outRes.op = simulator::operation::NE;
				} else {
					errorUnkInstr("Branch");
					return false;
				}

				return true;
			}

			// Branch with 1 register

			outRes.type = simulator::instrType::BRA1;
			if (!checkRegisters(1)) return false;

			outRes.rS = instr.rlist[0];

			if (name == "BGEZ") {
				outRes.op = simulator::operation::GEZ;
			} else if (name == "BGTZ") {
				outRes.op = simulator::operation::GTZ;
			} else if (name == "BLEZ") {
				outRes.op = simulator::operation::LEZ;
			} else if (name == "BLTZ") {
				outRes.op = simulator::operation::LTZ;
			} else {
				errorUnkInstr("Branch");
				return false;
			}

			return true;

		case 'J': // Inconditional jumps
			if (length != 1) {
				errorUnkInstr("Jump");
				return false;
			}

			if (!checkRegisters(0)) return false;
			if (!checkLabel()) return false;

			outRes.type = simulator::instrType::J;
			outRes.op = simulator::operation::NONE;
			outRes.labelOp = string((char *)instr.op.ptr);

			return true;
	}

	// Others: R-Type or NOP

	if (name == "NOP" || name == "NOOP") {
		if (!checkRegisters(0)) return false;
		if (!checkNoOP()) return false;

		outRes.type = simulator::instrType::NOP;
		outRes.op = simulator::operation::NONE;
		return true;
	}

	outRes.type = simulator::instrType::R3;

	if (name.starts_with("ADD")) {
		outRes.op = simulator::operation::ADD;
	} else if (name.starts_with("AND")) {
		outRes.op = simulator::operation::AND;
	} else if (name.starts_with("NOR")) {
		outRes.op = simulator::operation::NOR;
	} else if (name.starts_with("OR")) {
		outRes.op = simulator::operation::OR;
	} else if (name.starts_with("SUB")) {
		outRes.op = simulator::operation::SUB;
	} else if (name.starts_with("XOR")) {
		outRes.op = simulator::operation::XOR;
	}

	char modIdx = 3;

	switch (outRes.op) {
		case simulator::operation::ADD:
			if (length > 5) {
				errorUnkInstr("ADD");
				return false;
			}

			switch (name[3]) {
				case 'U':
					outRes.flags.mod = simulator::opMod::UNSIGNED;
					break;

				case 'I':
					outRes.flags.mod = simulator::opMod::IMMEDIATE;
					outRes.type = simulator::instrType::R2;
					break;

				case 0: // Regular ADD
					break;

				default:
					errorUnkInstr("ADD");
					return false;
			}

			if (length == 5) {
				if (name[3] != 'I' && name[4] != 'U') {
					errorUnkInstr("ADD");
					return false;
				}

				outRes.flags.mod = simulator::opMod::IMMEDIATE_UNSIGNED;
				// Last switch already set type to R2.
			}

			break;

		case simulator::operation::SUB:
			if (length > 4) {
				errorUnkInstr("ADD");
				return false;
			}

			switch (name[3]) {
				case 'U':
					outRes.flags.mod = simulator::opMod::UNSIGNED;
					break;

				case 0: // Regular SUB
					break;

				default:
					errorUnkInstr("SUB");
					return false;
			}

			break;

		case simulator::operation::OR:
			modIdx = 2;
		case simulator::operation::AND:
		case simulator::operation::NOR:
		case simulator::operation::XOR:
			if (length > modIdx + 1) {
				errorUnkInstr("R-Type");
				return false;
			}

			switch (name[modIdx]) {
				case 'I':
					outRes.flags.mod = simulator::opMod::IMMEDIATE;
					outRes.type = simulator::instrType::R2;
					break;

				case 0: // Regular operation
					break;

				default:
					errorUnkInstr("R-Type");
					return false;
			}

			break;

		default:
			errorUnkInstr("R-Type");
			return false;
	}

	if (outRes.type == simulator::instrType::R2) { // Immediate
		if (!checkRegisters(2)) return false;
		if (!checkImmediate()) return false;

		outRes.rT = instr.rlist[0];
		outRes.rS = instr.rlist[1];
		outRes.im = *(int *)instr.op.ptr;
	}

	// Regular R-Types

	if (!checkRegisters(3)) return false;
	if (!checkNoOP()) return false;

	outRes.rD = instr.rlist[0];
	outRes.rS = instr.rlist[1];
	outRes.rT = instr.rlist[2];

	return true;
}
} // namespace translator
