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




__kernel void dc_phase1
(
__global TraceSet *readTrace, 
__global TraceSet *writeTrace, 
__global int *readTo, 
__global int *writeTo, 
__global int *writeCount
)
{
#pragma OPENCL EXTENSION cl_amd_printf : enable
    
    size_t tid = get_global_id(0);
    
    for (int i = 0; i < readTrace[tid].size; i++) {
        int exist_in_write = 0;
        for (int j = 0; j < writeTrace[tid].size; j++) {
            if (readTrace[tid].indices[i] == writeTrace[tid].indices[j]) {
                exist_in_write = 1;
                break;
            }
        }
        
        if (!exist_in_write) {
            readTo[readTrace[tid].indices[i]] = 1;
        }
    }
    
    for (int i = 0; i < writeTrace[tid].size; i++) {
        writeTo[writeTrace[tid].indices[i]] = 1;
        writeCount[tid]++;
    }   
}


__kernel void reduce(
    __global int *buffer,
    __local int *scratch,
    __const int length,
    __global int *result
)
{
    int globalIndex = get_global_id(0);
    int accumulator = 0;

    // Loop sequentially over chunks of input vector
    while (globalIndex < length) 
    {
        int element = buffer[globalIndex];
        accumulator += element;
        globalIndex += get_global_size(0);
    }

    // Perform parallel reduction
    int lid = get_local_id(0);
    scratch[lid] = accumulator;
    barrier(CLK_LOCAL_MEM_FENCE);
    
    for(int offset = get_local_size(0) / 2; offset > 0; offset = offset / 2) {
        if (lid < offset) 
        {
            int other = scratch[lid + offset];
            int mine = scratch[lid];
            scratch[lid] = mine + other;
        }
        
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    if (lid == 0) {
        result[get_group_id(0)] = scratch[0];
    }
}

__kernel void dc_phase3
(
__global int *readTo,
__global int *writeTo, 
__global int *misspeculation,
__const int ARRAY_SIZE
)
{
	size_t tid = get_global_id(0);
    
    if (tid < ARRAY_SIZE && (readTo[tid] & writeTo[tid])) {
        *misspeculation = 1;
    }
}




