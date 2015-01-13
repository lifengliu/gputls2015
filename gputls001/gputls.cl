
// array size denotes : input and output size
//                      writeCount size
//                      writeTo size
//
// it is a multiple of wavefront size


#define TRACE_SIZE 5

#define READ_TRACE_SIZE 5

#define WRITE_TRACE_SIZE 5

#define NUM_VALUES 1024


typedef struct TraceNode {
    int size;
    int indices[TRACE_SIZE];  // record index, not address
    //for the case containing multiple arrays, maybe a code generator is needed.
} TraceNode;


float spec_read(size_t threadId, __global float *base_arr, int index, __global TraceNode *readTrace) {
    readTrace[threadId].indices[readTrace[threadId].size++] = index;
    return base_arr[index];
}


void spec_write(size_t threadId, __global float *base_arr, int index, float value, __global TraceNode *writeTrace) {
    writeTrace[threadId].indices[writeTrace[threadId].size++] = index;
    base_arr[index] = value;
}


/**
 *
 *   after speculative execution, 
 *   we use the read_trace and write_trace to perform dependency checking
 *
 */
__kernel void dependency_checking(__global TraceNode *readTrace, __global TraceNode *writeTrace, __global int *readTo, __global int *writeTo, __global int *writeCount, __global int *misspeculation) {
    
    size_t tid = get_global_id(0);
    
    for (int i = 0; i < readTrace[tid].size; i++) {
        char exist_in_write = 0;
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
    
    // parallel sum reduction on writeTo[] and writeCount[]
    for (size_t s = NUM_VALUES / 2; s > 0; s >>= 1) {
        if (tid < s) {
            writeTo[tid] += writeTo[tid + s];
            writeCount[tid] += writeCount[tid + s];
        }
        
        barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
    }
    
    
    if (tid == 0) { // writeTo[0] and writeCount[0] are the sums, check WAW
        if (writeTo[0] < writeCount[0]) {
            *misspeculation = 1;
        }
    }
    
    if (tid < NUM_VALUES && (readTo[tid] & writeTo[tid])) {
        *misspeculation = 1;
    }
    
    //printf("%d\n", writeTo[tid]);
    
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
__kernel void test_kernel(__global float *A, __global float *B, __global int *P, __global int *Q, __global TraceNode *readTrace, __global TraceNode *writeTrace) {
    
    size_t tid = get_global_id(0);
    
    //printf("%.2f ", A[tid]);
    
    //printf("tid %d P %d\n", tid, P[tid]);
    
    B[tid] = spec_read(tid, A, P[tid], readTrace); //100

    //printf("tid %d", tid);
    
    spec_write(tid, A, Q[tid], 100, writeTrace);
    
}



//kernel void square(__global float *input, __global float *output, __global TraceNode *readSet, __global TraceNode *writeSet) {
//    
//    size_t i = get_global_id(0);
//    char conflict;
//    //
//    //float a1 = spec_read(input + i, read_set, write_set, &conflict);
//    //float b1 = spec_read(input + i, read_set, write_set, &conflict);
//    
//    
//    barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
//    
//    //printf("%d a = %.2f b = %.2f address = %x conflict = %d\n", i, a1, b1, input + i, conflict);
//    
//    output[i] = input[i] * input[i];
//}















