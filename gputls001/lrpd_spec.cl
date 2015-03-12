/*


	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}



*/



typedef struct TraceSet {
    int size;
    int indices[5];  // record index, not address
} TraceSet;


float someCalculation(int CALC_SIZE) {
	float res = 0.5f;
	
	for (int i = 0; i < CALC_SIZE; i++) {
		res = sin(res);
	}
	
	return res;
}


/*
 *  loop example
 
	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}

 *
 *
 *
 */
 
__kernel void loop_kernel
(
__global float *a,
__global float *b,
__global float *c,
__global float *d,
__global int *Q,
__global int *P,
__global int *T,
__global TraceSet *read_trace_a,
__global TraceSet *write_trace_a,
__global TraceSet *read_trace_b,
__global TraceSet *write_trace_b,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
    size_t tid = get_global_id(0);
    
    read_trace_a[tid].size = 0;
    write_trace_a[tid].size = 0;
    
    read_trace_b[tid].size = 0;
    write_trace_b[tid].size = 0;
    
    
    
    write_trace_a[tid].indices[write_trace_a[tid].size++] = P[tid];
    read_trace_b[tid].indices[read_trace_b[tid].size++] = Q[tid];
    a[P[tid]] = b[Q[tid]] + c[Q[tid]];
    
    write_trace_b[tid].indices[write_trace_b[tid].size++] = T[tid];
    b[T[tid]] = 500;
    
    d[tid] = someCalculation(CALC_SIZE);  
}



