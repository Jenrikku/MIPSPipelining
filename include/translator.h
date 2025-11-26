#include "parseraux.h"
#include "simulator.h"
#include <ostream>

namespace translator
{
// Checks if the instruction is a variable definition directive.
bool isVarDef(instruction &instr);

// Verifies that the instruction is correct and translates it to a variable definition.
// Returns false if verification was not successful and prints an error to the stream.
bool toVarDef(instruction &instr, simulator::varDef &outRes, ostream err);

// Verifies that the instruction is correct and translates it to a more usable format.
// Returns false if verification was not successful and prints an error to the stream.
bool toInstruction(instruction &instr, simulator::instruction &outRes, ostream err);

// Frees all resources related to the underlying parser.
// Should only be called once and after all translation operations have been finished.
void endTranslation();
} // namespace translator
