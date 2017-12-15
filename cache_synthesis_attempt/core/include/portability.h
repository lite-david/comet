#ifndef PORTABILITY_H_
#define PORTABILITY_H_

#ifdef __VIVADO
    #include <ap_int.h>
    #define CORE_UINT(param) ap_uint<param>
    #define CORE_INT(param) ap_int<param>
    #define SLC(size,low) range(size + low -1, low)
    #define SET_SLC(low, value) range(low + value.length()-1,low) = value
#else
    #include <lib/ac_int.h>
    #define CORE_UINT(param) ac_int<param, false>
    #define CORE_INT(param) ac_int<param, true>
    #define SLC(size,low) slc<size>(low)
    #define SET_SLC(low, value) set_slc(low, value)
#endif

#endif /* For PORTABILITY_H_ */
