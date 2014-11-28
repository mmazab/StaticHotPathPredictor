#include "MemoryMap.hxx"
#include "InstructionMap.hxx"

using namespace Memory;
using namespace Instruction;

extern "C" {
    int main() {
	MemoryMap<uint8_t> *map = new MemoryMap<uint8_t>();
	InstructionMap<uint8_t> *im = new InstructionMap<uint8_t>(10);
	return 0;
    }
}
