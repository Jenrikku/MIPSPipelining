#include <cstdint>
#include <string>
#include <unordered_map>

using namespace std;

namespace pipeline
{
namespace interpreter
{

class executer
{
  public:
	executer();

  private:
	unordered_map<string, void (*)(uint16_t, uint16_t, uint16_t, string)> instrImpl;
};

} // namespace interpreter
} // namespace pipeline