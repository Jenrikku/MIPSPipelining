#include "simulator.h"

namespace simulator
{
bool memory::add(varDef def)
{
	if (labelMap.contains(def.label)) return false;

	labelMap[def.label] = internalMem.size() - 1;

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

	if (def.type == simulator::varType::ARRAY) {
		uint n = dataSize * def.value;
		internalMem.resize(internalMem.size() + n, 0);
		return true;
	}

	for (char i = 0; i < dataSize; i++) {
		char v = def.value >> i * 8;
		internalMem.push_back(v);
	}

	return true;
}

void memory::shrink()
{
	internalMem.shrink_to_fit();
}
} // namespace simulator
