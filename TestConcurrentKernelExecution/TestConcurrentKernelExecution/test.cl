#pragma OPENCL EXTENSION cl_amd_printf : enable
__kernel void testcalc(__global int *arr1, __global int *out, __const int N, __const int calcSize) {
	size_t tid = get_global_id(0);
	
	int i;
	if (tid < N) {
		for (i = 0; i < calcSize; i++) {
			out[tid] = (arr1[tid] * i) % 3;
		}
	}
}


__kernel void testcalc1(__global int *arr1, __global int *out, __const int N, __const int calcSize) {
	size_t tid = get_global_id(0);
	
	if (tid < N) {
		int i;
		for (i = 0; i < calcSize; i++) {
			if (arr1[tid] % 2 == 0) {
				if (i % 2 == 0) {
					out[(tid + 1) % N] += i;
					out[(tid + 1) % N] %= 3;
				} else {
					out[(tid + 1) % N] -= i;
					out[(tid + 1) % N] %= 3; 
				}
			}
		}
	}
}


__kernel void testatom(__global int *arr1, __global int *out, __const int N, __const int calcSize) {


	size_t tid = get_global_id(0);
	
	/*if (tid == 0) {
		printf("%s\n", "fuck");
	}*/

	if (tid < N) {
	    int j;
		if (arr1[tid] % 2 == 0) {
		    for (j = 0; j < calcSize; j++) {
				atomic_inc(&out[tid]);  
				out[tid] %= 3;
			}
		}
	}

}