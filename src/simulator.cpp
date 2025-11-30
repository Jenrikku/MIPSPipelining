#include "simulator.h"

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

	int index = internalMem.size() - 1;

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
} // namespace simulator
