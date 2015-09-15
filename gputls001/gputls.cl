
// array size denotes : input and output size
//                      writeCount size
//                      writeTo size
//


#define TRACE_SIZE 5
#define READ_TRACE_SIZE 5
#define WRITE_TRACE_SIZE 5

typedef struct TraceNode {
    int size;
    int indices[TRACE_SIZE];  // record index, not address
    //for the case containing multiple arrays, maybe a code generator is needed.
} TraceNode;


float func(int i, int FUNC_LOOP_NUM) {
	float res = 0.0f;

	for (int p = 1; p <= FUNC_LOOP_NUM; p++) {
		res += (p + i) % 5;
	}

	return res;
}

float spec_read(size_t threadId, __global float *base_arr, int index, __global TraceNode *readTrace) {
	readTrace[threadId].indices[readTrace[threadId].size] = index;
	readTrace[threadId].size++;
	
	return base_arr[index];
}


void spec_write(size_t threadId, __global float *base_arr, int index, float value, __global TraceNode *writeTrace) {
    writeTrace[threadId].indices[writeTrace[threadId].size] = index;
    writeTrace[threadId].size++;
    
    base_arr[index] = value;
}


/**
 *
 *   after speculative execution, 
 *   we use the read_trace and write_trace to perform dependency checking
 *
 */
__kernel void dependency_checking_phase_one
(
__global TraceNode *readTrace, 
__global TraceNode *writeTrace, 
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


__kernel void dependency_checking_phase_three
(
__global int *readTo,
__global int *writeTo, 
__global int *misspeculation,
__const int NUM_VALUES
)
{
	size_t tid = get_global_id(0);
    
    /*if (tid == 0) { // writeTo[0] and writeCount[0] are the sums, check WAW
        if (writeTo[0] < writeCount[0]) {
            *misspeculation = 1;
        }
    }*/
    
    if (tid < NUM_VALUES && (readTo[tid] & writeTo[tid])) {
        *misspeculation = 1;
    }
    
}

/*
 *
 *   originally
 *   this is a loop  
 *
 *    for int i = 0; i < NUM_VALUES; i ++
 *         B[i] = A[P[i]]
 *         A[Q[i]] = 100
 *
 *
 *   it will be converted to the kernel below
 *
 *
 *
 */
__kernel void test_kernel
(
__global float *A,
__global float *B,
__global int *P,
__global int *Q,
__const int FUNC_LOOP_NUM,
__global TraceNode *readTrace,
__global TraceNode *writeTrace
) 

{
    #pragma OPENCL EXTENSION cl_amd_printf : enable
    
    size_t tid = get_global_id(0);
    
    readTrace[tid].size = 0;
    
    writeTrace[tid].size = 0;
    
    spec_write(tid, A, P[tid], func(tid, FUNC_LOOP_NUM), writeTrace);
        
    B[tid] = spec_read(tid, A, Q[tid], readTrace); 

    
}
















