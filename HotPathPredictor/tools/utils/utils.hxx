#ifndef UTILS_HXX
#define UTILS_HXX

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

    int display_char(FILE *fp, int what);
    void write_ltrace(FILE *fp, const char *data, size_t len);
    void write_strace(FILE *fp, const char *data, size_t len);
    void print_backtrace(FILE *fp, size_t max_size);

#ifdef __cplusplus
}
#endif

#endif
