//**************************************************************************
// Median filter bencmark
//--------------------------------------------------------------------------
//
// This benchmark performs a 1D three element median filter. The
// input data (and reference data) should be generated using the
// median_gendata.pl perl script and dumped to a file named
// dataset1.h.

#include "median.h"
#include "dataset1.h"
#include "custom_inst.h"
#include "util.h"
// Main
int main( )
{
	int results_data[DATA_SIZE];

  // Do the filter
	int i;
	CUSTOMX_R_R_R(0,i,0,0,0)
   	median( DATA_SIZE, input_data, results_data );
	CUSTOMX_R_R_R(0,i,0,0,0)
	return verify(DATA_SIZE,verify_data,results_data);
}

