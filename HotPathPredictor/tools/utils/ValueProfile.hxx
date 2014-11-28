#ifndef VALUE_PROFILING_H
#define VALUE_PROFILING_H

#include <inttypes.h>
#include <iostream>
#include <ostream>

#include "Profile.hxx"

#include <map>
#include <vector>
#include <iterator>

using namespace std;

namespace Profiling {

    class ValueCount {
    public:
	uint64_t value;
	uint64_t count;

	ValueCount() : count(0) {}
    };


    class ValueProfile {
    private:
	static const uint64_t MAX_VALUES = 5;

	ValueCount values[MAX_VALUES];

	uint64_t count;

    public:
	ValueProfile() : count(0) {
            memset(this->values, 0, sizeof(this->values));
        }

	void increment(const uint64_t value) {
	    this->count++;
	    
	    for (uint32_t i = 0; i < MAX_VALUES; i++) {
		if (this->values[i].value == value) {
		    this->values[i].count++;
		    return;
		}
	    }
	    
	    for (uint32_t i = 0; i < MAX_VALUES; i++) {
		if (this->values[i].count == 0) {
		    this->values[i].value = value;
		    this->values[i].count = 1;
		    return;
		}
	    }
	}

	friend ostream &operator<<(ostream &stream, const ValueProfile &vp);
    };

    ostream &operator<<(ostream &stream, const ValueProfile &vp) {
	stream<<vp.count<<" : ";
	if (vp.count == 0)
	    return stream;
	    
	for (uint32_t j = 0; j < ValueProfile::MAX_VALUES; j++) {
	    if (vp.values[j].count != 0) {
		stream<<vp.values[j].value<<" "<<vp.values[j].count<<" : ";
	    }
	}

	return stream;
    }

    template <int maxTrackedDistance = DEFAULT_TRACKED_DISTANCE>
    class ValueProfiler : public KeyDistanceProfiler<ValueProfile, maxTrackedDistance> {
    public:
	ValueProfiler(const uint32_t num_instrs) 
	    : KeyDistanceProfiler<ValueProfile, maxTrackedDistance>(num_instrs) {}

	void increment(const Dependence &dep, const uint64_t value) {
	    ValueProfile & valueProfile = this->getProfile(dep);
	    valueProfile.increment(value);
	}

	template<int S>
	friend ostream &operator<<(ostream &stream, const ValueProfiler<S> &vp);
    };

    template<int S>
    ostream &operator<<(ostream &stream, const ValueProfiler<S> &vp) {
	stream<<"BEGIN Value Profile"<<endl;
	stream<<((KeyDistanceProfiler<ValueProfile, S> &) vp);
	stream<<"END Value Profile"<<endl;
	return stream;
    }
}

#endif
