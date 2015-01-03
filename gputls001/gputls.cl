#define TRACE_SIZE 5
#define ARRAY_SIZE 1024

// array size denotes : input and output size
//                      writeCount size
//                      writeTo size
//
// it is a multiple of wavefront size

typedef struct TraceNode {
    global int size;
    global float *address[TRACE_SIZE];
} TraceNode;


float multiplyfunc(float a, float b) {
    return a * b;
}


float spec_read(size_t threadId, global float *address, global TraceNode *readTrace) {
    readTrace[threadId].address[readTrace[threadId].size++] = address;
    return *address;
}

void spec_write(size_t threadId, global float *address, float value, TraceNode *writeTrace) {
    writeTrace[threadId].address[writeTrace[threadId].size++] = address;
    *address = value;
}

kernel void square(global float *input, global float *output, global TraceNode *readSet, global TraceNode *writeSet) {
    
    size_t i = get_global_id(0);
    char conflict;
    //
    //float a1 = spec_read(input + i, read_set, write_set, &conflict);
    //float b1 = spec_read(input + i, read_set, write_set, &conflict);
    
    
    barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
    
    //printf("%d a = %.2f b = %.2f address = %x conflict = %d\n", i, a1, b1, input + i, conflict);
    
    output[i] = input[i] * input[i];
}

/*
 before dependency checking
 perform sum reduction on writeCount and writeTo
*/

kernel void dependency_checking(global TraceNode *readTrace, global TraceNode *writeTrace, global int *readTo, global int *writeTo, global int *writeCount, global int *misspeculation) {
    
    size_t tid = get_global_id(0);
    
    for (int i = 0; i < readTrace[tid].size; i++) {
        char exist_in_write = 0;
        for (int j = 0; j < writeTrace[tid].size; j++) {
            if (readTrace[tid].address[i] == writeTrace[tid].address[j]) {
                exist_in_write = 1;
                break;
            }
        }
    }
    
    
    // parallem sum reduction on writeTo[] and writeCount[]
    for (size_t s = ARRAY_SIZE / 2; s > 0; s >>= 1) {
        if (tid < s) {
            writeTo[tid] += writeTo[tid + s];
            writeCount[tid] += writeCount[tid + s];
        }
        
        barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
    }
    
    if (tid == 0) { // writeTo[0] and writeCount[0] are the sums, check WAW
        if (writeTo[0] < writeCount[0]) {
            //misspeculation
            atomic_inc(misspeculation);
        }
    }
    
    if (tid < ARRAY_SIZE && (readTo[tid] & writeTo[tid])) {
        atomic_inc(misspeculation);
    }
    
    printf("%d\n", writeTo[tid]);
    
}













