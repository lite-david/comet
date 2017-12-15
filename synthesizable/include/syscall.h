#include "portability.h"

CORE_UINT(32) solveSysCall(CORE_UINT(32) syscallId, CORE_UINT(32) arg1, CORE_UINT(32) arg2,CORE_UINT(32) arg3,
 CORE_UINT(32) arg4, CORE_UINT(2) *sys_status);