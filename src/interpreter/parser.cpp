#include "interpreter/parser.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace pipeline
{
namespace interpreter
{

instruction *parseLine(string line)
{
	istringstream stream = istringstream(line);

	string buf;
	instruction *res = new instruction();

	for (int i = 0; stream >> buf; i++) {
		if (i == 0 && buf.ends_with(':')) {
			buf.pop_back();
			res->label = buf;
			i--;
			continue;
		}

		if (buf.ends_with(',')) buf.pop_back(); // Remove trailing comma

		if (i == 0) {
			// Transform instruction operation to uppercase
			transform(buf.begin(), buf.end(), buf.begin(), [](unsigned char c) { return toupper(c); });
			res->op = buf;
		} else if (!isdigit(buf[1])) { // FIXME: Registers start with $ or R
			res->labelOp = buf;
		} else {
			uint16_t r = atoi(buf.c_str());

			switch (i) {
				case 1:
					res->r0 = r;
					break;

				case 2:
					res->r1 = r;
					break;

				case 3:
					res->r2 = r;
					break;
			}
		}
	}

	return res;
}

} // namespace interpreter
} // namespace pipeline