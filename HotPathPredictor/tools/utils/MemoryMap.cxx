#define __STDC_FORMAT_MACROS

#include "MemoryMap.hxx"
#include <stdio.h>

FILE *Memory::debug_log = stderr;

int Memory::max_seqno = 0;

uint64_t Memory::debug_address = 0x0;

void Memory::memory_map_init(FILE *debug) {
    debug_log = debug;
}

bool Memory::filter_address(uint64_t addr) {
    return ((addr & 0xF000000000000000) == 0x2000000000000000);
    //return ((addr & 0xFFFFFFFFFFFFF000) == 0x2000000002c46000);
}

bool Memory::check_address_in_range(uint64_t addr, uint64_t base, uint64_t length) {
    return (addr >= base) && (addr < (base + length));
}
