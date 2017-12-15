// See LICENSE for license details.

//**************************************************************************
// Vector-vector add benchmark
//--------------------------------------------------------------------------
//
// This benchmark uses adds to vectors and writes the results to a
// third vector. The input data (and reference data) should be
// generated using the vvadd_gendata.pl perl script and dumped
// to a file named dataset1.h.
 

//--------------------------------------------------------------------------
// Input/Reference Data

#include "dataset1.h"
#include "custom_inst.h"
#include "util.h"
//--------------------------------------------------------------------------
// vvadd function

void vvadd( int n, int a[], int b[], int c[] )
{
  int i;
  for ( i = 0; i < n; i++ )
    c[i] = a[i] + b[i];
}

//--------------------------------------------------------------------------
// Main

int main()
{
  int results_data[DATA_SIZE];

  // Do the vvadd
  int i;
  CUSTOMX_R_R_R(0,i,0,0,0);
  vvadd( DATA_SIZE, input1_data, input2_data, results_data );
  CUSTOMX_R_R_R(0,i,0,0,0);
 
  //verify
  return verify(DATA_SIZE,results_data,verify_data);	
}
