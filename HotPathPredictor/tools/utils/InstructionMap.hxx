#ifndef INSTRUCTION_HXX
#define INSTRUCTION_HXX

#include <inttypes.h>

using namespace std;

namespace Instruction {

    template<class T>
    class InstructionMap {
    private:
	const uint32_t max_instrs;

	T const *instrs;

    public:
	InstructionMap(const uint32_t num_instrs) : max_instrs(num_instrs) {
	    this->instrs = new T[this->max_instrs];
	}

	~InstructionMap(void) {
	    delete[] this->instrs;
	}

	T& operator[](const uint32_t id) {
	    if (id > max_instrs) {
		fprintf(stderr, "Instruction id too large\n");
		abort();
	    }
	    return instrs[id];
	}

	const T& operator[](const uint32_t id) const {
	    if (id > max_instrs) {
		fprintf(stderr, "Instruction id too large\n");
		abort();
	    }
	    return instrs[id];
	}
    };
}

#endif
