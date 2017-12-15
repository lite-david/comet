#include <portability.h>
#define DRAM_WIDTH 8
#define DRAM_SIZE 100000000

void doStep(CORE_UINT(32) pc, CORE_UINT(32) nbcycle, CORE_UINT(DRAM_WIDTH) dram[DRAM_SIZE]);