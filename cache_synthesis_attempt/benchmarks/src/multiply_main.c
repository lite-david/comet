#include "multiply.h"
#include "custom_inst.h"
#include "dataset1.h"
#include "util.h"


int main(){
	int i;
	int results_data[DATA_SIZE];

	int j;
	CUSTOMX_R_R_R(0,j,0,0,0)
	for(i = 0; i< DATA_SIZE;i++){
		results_data[i] = multiply(input_data1[i], input_data2[i]);
	}
	CUSTOMX_R_R_R(0,j,0,0,0)
	return verify(DATA_SIZE,results_data,verify_data);	
}
