#ifndef  __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "debug_new.hxx"

#include <map>
#include <ext/malloc_allocator.h>

#include "utils.hxx"

using namespace std;
using namespace __gnu_cxx;

class VoidHash {
public:
    int operator()(const void *ptr) const {
        return (intptr_t) ptr;
    }
};

typedef map<void *, size_t, std::greater<void *>, malloc_allocator<size_t> > PointerMap;

static PointerMap pointerMap;

void * operator new(size_t size) throw (std::bad_alloc) {
    void *result = malloc(size);
    if (result == NULL)
        return result;

#ifdef DETAILED_DEBUG_NEW
    fprintf(stderr, "Allocation %p [%lu]\n", result, size);
    print_backtrace(stderr, 10);
    fprintf(stderr, "\n");
#endif

    pointerMap[result] = size;
    return result;
}

void * operator new[](size_t size) throw (std::bad_alloc) {
    return operator new(size);
}

void operator delete(void *ptr) throw () {
    PointerMap::iterator iter = pointerMap.find(ptr);
    if (iter == pointerMap.end()) {
        fprintf(stderr, "Unable to find allocation to delete for %p\n", ptr);
        abort();
    }

#ifdef DETAILED_DEBUG_NEW
    fprintf(stderr, "Deallocating %p\n", ptr);
    print_backtrace(stderr, 10);
    fprintf(stderr, "\n");
#endif

    pointerMap.erase(iter);
    free(ptr);
}

void operator delete[](void *ptr) throw () {
    operator delete(ptr);
}

void print_addresses(FILE *fp) {
    fprintf(fp, "Allocated Addresses\n");
    for (PointerMap::iterator iter = pointerMap.begin(); iter != pointerMap.end(); iter++) {
        if (iter->second != 0) {
            char *endptr = ((char *) iter->first) + iter->second - 1;
            fprintf(fp, "%p-%p [%lu]\n", iter->first, endptr, iter->second);
        } else {
            fprintf(fp, "%p [%lu]\n", iter->first, iter->second);
        }
    }
}

void check_address(void *ptr, FILE *fp) {
    PointerMap::iterator iter = pointerMap.lower_bound(ptr);
    if (iter == pointerMap.end())
        return;

    if ((((char *) iter->first) + iter->second) > ptr) {
        char *endptr = ((char *) iter->first) + iter->second - 1;
        fprintf(fp, "Address Check Failure: %p in allocation (%p, %p) [%" PRIu64 "] \n", ptr, iter->first, endptr, iter->second);
        abort();
    }
}
