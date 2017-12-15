// vim: set ts=4 nu ai:
#ifndef CACHE_H
#define CACHE_H

#include <portability.h>
#define SETS 64
#define SETBITS 6 // log2(SETS)
#define LATENCY 30
#define CACHEBLOCKBYTES 64
#define IDBITS 6 // log2(CACHEBLOCKBYTES)
#define TAGBITS 20 // 32 - SETBITS - IDBITS

#define BYTE_WIDTH 0
#define HALF_WORD_WIDTH 1
#define WORD_WIDTH 3

#define HALF_WORD_MASK 1
#define WORD_MASK 2

#define LOAD 0
#define STORE 1

struct cache_index{
	CORE_UINT(TAGBITS) tag;
	CORE_UINT(1) dirtybit;
	CORE_UINT(1) invalid;
};

struct Cache{
	CORE_UINT(8) data[SETS][CACHEBLOCKBYTES];
	CORE_UINT(32) dram_read_address;
	CORE_UINT(32) dram_write_address;
	CORE_UINT(IDBITS+1) id_counter_1;
	CORE_UINT(IDBITS+1) id_counter_2;
	struct cache_index index[SETS];
};

#endif /* CACHE_H */
