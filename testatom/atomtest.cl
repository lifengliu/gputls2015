__kernel void atom_test(__global int *idx_array, __global int *buf_array, __const int N) {	
	size_t tid = get_global_id(0);
	
	if (tid < N) {
		atomic_inc(&buf_array[idx_array[tid]]);
		//buf_array[idx_array[tid]]++;
	}
}

