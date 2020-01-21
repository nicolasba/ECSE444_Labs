#include <stdev.h>
#include <math.h>

void c_stdev(float *f_arr, int N, float *stdev){
		
		float mean = 0;
		float sq_sum = 0;
	
		//Compute the mean
		for (int i = 0; i < N; i++) {
			mean += *(f_arr + i);
		}
		mean /= N;
		
		//Compute the stdev
		for (int i = 0; i < N; i++) {
			
			float x = *(f_arr + i) - mean;
			sq_sum += x * x;
		}
		sq_sum /= N - 1;
		
		*stdev = pow(sq_sum, 0.5);
}
