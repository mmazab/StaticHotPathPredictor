#include "utils.hxx"
#include <stdio.h>
#include <ctype.h>
#include <execinfo.h>
#include <malloc.h>
#include <alloca.h>
#include <string.h>

/**
 * From ltrace source code - v0.5
 */
int display_char(FILE *output, int what) {
    switch (what) {
    case -1:
	return fprintf(output, "EOF");
    case '\r':
	return fprintf(output, "\\r");
    case '\n':
	return fprintf(output, "\\n");
    case '\t':
	return fprintf(output, "\\t");
    case '\b':
	return fprintf(output, "\\b");
    case '\\':
	return fprintf(output, "\\\\");
    default:
	if ((what < 32) || (what > 126)) {
	    return fprintf(output, "\\%03o", (unsigned char)what);
	} else {
	    return fprintf(output, "%c", what);
	}
    }
}

void write_ltrace(FILE *fp, const char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
	display_char(fp, data[i]);
    }
}

void write_strace(FILE *fp, const char *data, size_t len) {
    static char outstr[80];
    unsigned int i, j;
        
    for (i = 0; i < len; i += 16) {
	char * s = outstr;
	sprintf(s, " | %05x ", i);
	s += 9;
	for (j = 0; j < 16; j++) {
	    if (j == 8)
		*s++ = ' ';
	    if ((i + j) < len) {
		sprintf(s, " %02x", data[i + j]);
		s += 3;
	    } else {
		*s++ = ' '; *s++ = ' '; *s++ = ' ';
	    }
	}
	*s++ = ' '; *s++ = ' ';
	for (j = 0; j < 16; j++) {
	    if (j == 8)
		*s++ = ' ';
	    if ((i + j) < len) {
		if (isprint(data[i + j]))
		    *s++ = data[i + j];
		else
		    *s++ = '.';
	    } else {
		*s++ = ' ';
	    }
	}
	fprintf(fp, "%s |\n", outstr);
    }
}

void print_backtrace(FILE *fp, size_t max_size) {
    // We're going to ignore the backtrace item for this function, so increment by one
    max_size++;

    void **array = (void **) alloca(max_size * sizeof(void *));
    size_t size;
    char **strings;
    size_t i;
    
    size = backtrace (array, max_size);
    strings = backtrace_symbols (array, size);
    
    fprintf (fp, "Backtrace: [%zd frames]:\n", size);
    
    // Skip the frame from this function
    for (i = 1; i < size; i++)
        fprintf (fp, "  %2zu: %s\n", i, strrchr(strings[i], '/'));

    free(strings);
    fflush(fp);
}
