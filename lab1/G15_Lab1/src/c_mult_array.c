#include <mult_array.h>

void c_mult_array(float *f_arr_a, float *f_arr_b, float *f_res, int N) {
		
		for (int i = 0; i < N; i++) {
			*(f_res + i) = *(f_arr_a + i) * *(f_arr_b + i);
		}
}
