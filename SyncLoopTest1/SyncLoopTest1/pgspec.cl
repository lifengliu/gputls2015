/*


	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}


#pragma scop
for (int i = 0; i < N; i++) {
    if (c[i] > 0) {
        a[Q[i]] = comp1(size1) + a[P[i]];
    } else {
        a[Q[i]] = comp2(size2);
    }
}
#pragma endscop


*/





typedef struct TraceSet {
    int size;
    int indices[5];  // record index, not address
} TraceSet;


int comp1(int CALC_SIZE) {	
	int res = 0;
	for (int p = 1; p <= CALC_SIZE; p++) {
		res = (p + res) % 1047;
	}

	return res;
}

int comp2(int CALC_SIZE) {	
	int res = 0;
	for (int p = CALC_SIZE; p >= 1; p--) {
		res = (p + res) % 1031;
	}

	return res;
}

__kernel void loop_task_spec_kernel(
__global int *P, 
__global int *Q, 
__global int *a, 
__global int *c, 
__const int N, 
__const int size1, 
__const int size2,
__global TraceSet *read_trace_a,
__global TraceSet *write_trace_a
)
{
	int tid = get_global_id(0);

	int b0 = get_group_id(0);
    int t0 = get_local_id(0);

	read_trace_a[tid].size = 0;
    write_trace_a[tid].size = 0;

    #define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576)
      if (N >= t0 + c0 + 1)
        if (c[t0 + c0] > 0) {
		     write_trace_a[tid].indices[write_trace_a[tid].size++] = Q[t0 + c0];
			 read_trace_a[tid].indices[read_trace_a[tid].size++] = P[t0 + c0];
			 a[Q[t0 + c0]] = comp1(size1) + a[P[t0 + c0]];
        } else {
		     write_trace_a[tid].indices[write_trace_a[tid].size++] = Q[t0 + c0];
			 a[Q[t0 + c0]] = comp2(size2);
        }
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




