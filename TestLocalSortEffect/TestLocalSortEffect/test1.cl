
__kernel void test1
(
__global int *a, 
__const int N
)
{
    int tid = get_global_id(0);
	if (tid < N) {
		a[tid] = tid;
	}
}

