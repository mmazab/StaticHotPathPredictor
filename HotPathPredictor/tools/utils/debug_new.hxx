#ifndef DEBUG_NEW_HXX
#define DEBUG_NEW_HXX

#include <inttypes.h>
#include <stdio.h>
#include <new>

using namespace std;

void * operator new (size_t size) throw (std::bad_alloc);
void * operator new[] (size_t size) throw (std::bad_alloc);

void operator delete (void *ptr) throw ();
void operator delete[] (void *ptr) throw ();

#ifdef __cplusplus
extern "C" {
#endif

    void print_addresses(FILE *fp);
    void check_address(void *ptr, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
