#define TRACE_SIZE 5

typedef struct TraceNode {
    global int size;
    global float *address[TRACE_SIZE];
} TraceNode;


float multiplyfunc(float a, float b) {
    return a * b;
}


float spec_read(size_t threadId, global float *address, global TraceNode *readSet, global TraceNode *writeset) {
    
    readSet[threadId].address[readSet[threadId].size++] = address;
    return *address;
    
}


kernel void square(global float *input, global float *output, global struct TraceNode *readSet, global struct TraceNode *writeSet) {
    
    size_t i = get_global_id(0);
    char conflict;
    //
    //float a1 = spec_read(input + i, read_set, write_set, &conflict);
    //float b1 = spec_read(input + i, read_set, write_set, &conflict);
    
    
    barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
    
    //printf("%d a = %.2f b = %.2f address = %x conflict = %d\n", i, a1, b1, input + i, conflict);
    
    output[i] = input[i] * input[i];
}
















