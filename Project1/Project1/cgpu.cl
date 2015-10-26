#define DUMB_COMPUTATION_NUM 10000

typedef struct TraceSet {
    int size;
    int indices[5];  // record index, not address
} TraceSet;

int force_GPU(int Xi, int Xj)
{
	int sum = 0;
	int i = 0;
	for(i = 0; i < DUMB_COMPUTATION_NUM; i++)
	{
		sum += (Xi*Xi + Xj*Xj + 2*Xi*Xj + (int)sqrt( (float) Xi) /*+ pow((double)(Xi*Xj), 2)*/);	
	}
	return sum;
}



__kernel void ourdc
(
__global int* d_X, 
__global int* d_Y, 
__global int* d_LeftNode, 
__global int* d_RightNode,
__global int* buffer,
__global int* raceFlag,
__const int NumEdges
) 
{
	int global_thread_ID = get_global_id(0);
	int v = -1;

	if(global_thread_ID < NumEdges) {
		int n1 = d_LeftNode[global_thread_ID];
		int n2 = d_RightNode[global_thread_ID];
		
		//n1 n2 WAW
		
		int v = atomic_xchg(&buffer[n1], global_thread_ID);
		*raceFlag |=  !(v == 0 || v == global_thread_ID);

		v = (v != global_thread_ID) ? -1 : atomic_xchg(&buffer[n2], global_thread_ID);
		*raceFlag |= !(v == 0 || v == global_thread_ID);

		//buffer[n1] = global_thread_ID;
		//buffer[n2] = global_thread_ID;

		//d_Y[n1] += force_GPU(d_X[n1], d_X[n2]);
		//d_Y[n2] += force_GPU(d_X[n1], d_X[n2]);
	}

}

__kernel void GPU_kernel1
(
__global int* d_X, 
__global int* d_Y, 
__global int* d_LeftNode, 
__global int* d_RightNode,
__const int NumEdges
) 
{
	int global_thread_ID = get_global_id(0);

	if(global_thread_ID < NumEdges) {
		int n1 = d_LeftNode[global_thread_ID];
		int n2 = d_RightNode[global_thread_ID];
		
		d_Y[n1] += force_GPU(d_X[n1], d_X[n2]);
		d_Y[n2] += force_GPU(d_X[n1], d_X[n2]);
	}
}



// ----------------------------------------------------------------------------------------------------------------------------

__kernel void pg_loop_kernel(
__global int* d_X, 
__global int* d_Y, 
__global int* d_LeftNode, 
__global int* d_RightNode,
__const int NumEdges,
__global TraceSet *read_trace_Y,
__global TraceSet *write_trace_Y
)
{
	int tid = get_global_id(0);

	read_trace_Y[tid].size = 0;
    write_trace_Y[tid].size = 0;

	int global_thread_ID = get_global_id(0);

	if(global_thread_ID < NumEdges) {
		int n1 = d_LeftNode[global_thread_ID];
		int n2 = d_RightNode[global_thread_ID];
		
		//n1 n2 WAW
		
		write_trace_Y[tid].indices[write_trace_Y[tid].size++] = n1;
		d_Y[n1] += force_GPU(d_X[n1], d_X[n2]);
		
		write_trace_Y[tid].indices[write_trace_Y[tid].size++] = n2;
		d_Y[n2] += force_GPU(d_X[n1], d_X[n2]);
		
		
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