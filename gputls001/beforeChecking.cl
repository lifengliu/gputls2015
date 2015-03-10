/*

	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = 600;
	    someCalculation();
	}

*/

float someCalculation(int CALC_SIZE) {
	float res = 0.5f;
	
	for (int i = 0; i < CALC_SIZE; i++) {
		res = sin(res);
	}
	
	return res;
}



__kernel void loop_kernel
(
__global float *a,
__global float *b,
__global float *c,
__global float *d,
__global int *Q,
__global int *P,
__global int *T,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
    size_t tid = get_global_id(0);
    a[P[tid]] = b[Q[tid]] + c[Q[tid]];
    b[T[tid]] = 500;
    d[tid] = someCalculation(CALC_SIZE);   
}



//__kernel void mark    mark P 

__kernel void markwriteP
(
__global int *buffer,
__global int *P,
__global int *raceFlag
)
{
    size_t tid = get_global_id(0);
    int writeTimes = atomic_inc(&buffer[P[tid]]);
    if (writeTimes > 0) {
    	*raceFlag = 1;
    }
}


//     mark Q & mark T
__kernel void markwriteT
(
__global int *buffer,
__global int *T,
__global int *raceFlag
)
{
    size_t tid = get_global_id(0);
    int writeTimes = atomic_inc(&buffer[T[tid]]);
    
    if (writeTimes > 0) {
    	*raceFlag = 1;
    }
}



__kernel void markReadQ
(
__global int *writeBuffer,
__global int *Q,
__global int *raceFlag
)
{
    // the buffer contains write information
    
    size_t tid = get_global_id(0);
    
    if ( writeBuffer[Q[tid]] > 0) {
        *raceFlag = 1;
    }
}




