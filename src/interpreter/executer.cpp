#include "interpreter/executer.h"

namespace pipeline
{
namespace interpreter
{

#pragma region Instruction Implementations

void add(uint16_t r0, uint16_t r1, uint16_t r2, string label) {}

#pragma endregion

executer::executer()
{
	instrImpl = unordered_map<string, void (*)(uint16_t, uint16_t, uint16_t, string)>();
}

} // namespace interpreter
} // namespace pipeline