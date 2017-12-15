#include <qsort.h>
int partition(int low,int hi){
	int pivot = A[hi];
	int i = low-1,j;
	int temp;
	for(j = low;j<hi;j++){
		if(A[j] < pivot){
			i = i+1;
			temp = A[i];
			A[i] = A[j];
			A[j] = temp;
		}
	}
	if(A[hi] < A[i+1]){
		temp = A[i+1];
		A[i+1] = A[hi];
		A[hi] = temp;
	}
	return i+1;
}

void  qsort(int low, int hi){
	if(low < hi){
		int p = partition(low, hi);
		qsort(low,p-1);
		qsort(p+1,hi);
	}
}

int main(){
	qsort(0,SIZE-1);
}
