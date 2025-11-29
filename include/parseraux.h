#include <iostream>
#include <vector>

using namespace std;

namespace parser
{
enum optype { OPNONE = 0, OPLABEL, OPINDIRECT, OPIM };

typedef struct indirect {
	int im;
	int reg;
} indirect;

typedef struct operand {
	optype type;
	void *ptr;
} operand;

typedef struct instruction {
	const char *label;
	const char *name;
	const int *rlist;
	int rcount;
	operand op;
} instruction;

std::vector<instruction> parse(istream *in);
void freeResources();
} // namespace parser