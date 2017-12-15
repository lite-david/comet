#include "portability.h"
#include <cache.h>

void doStep(CORE_UINT(32) pc, CORE_UINT(32) nbcycle, Cache* ICache,
	Cache* Dcache, CORE_INT(32) dm_out[8192]);//, CORE_INT(32) debug_arr[200]);
