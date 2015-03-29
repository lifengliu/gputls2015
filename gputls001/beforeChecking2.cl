/*


如果 condition 变量 是immutable的 就拆
否则 全局 check 过了check再拆


for int i = 1 to 100000 do {
    
    a[P[i]] = b[Q[i]];

    if (c[i] > 0) { //read 

        a[Q[i]] = b[P[i]] * b[Q[i]];
    
    } else {
        
        b[P[i]] = a[Q[i]];
    
    }

    a[P[i]] = some_calculation();
}


*/


typedef struct IndexNode {
    int index;
    int condVal;
} IndexNode;


float some_calculation(int CALC_SIZE) {
	float res = 0.5f;
	
	for (int i = 0; i < CALC_SIZE; i++) {
		res = sin(res);
	}
	
	return res;
}



__kernel void loop_kernel_origin
(
__global float *a,
__global float *b,
__global float *c,
__global int *Q,
__global int *P,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
    int i = get_global_id(0);
    
    a[P[i]] = b[Q[i]];

    if (c[i] > 0) {
        a[Q[i]] = b[P[i]] * b[Q[i]];
    } else {
        b[P[i]] = a[Q[i]];
    }

    a[P[i]] = some_calculation(CALC_SIZE);
}



__kernel void loop_kernel_0
(
__global float *a,
__global float *b,
__global int *Q,
__global int *P,
__global IndexNode *index_node,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
    //else
    
    int i = index_node[get_global_id(0)].index; 
    
    a[P[i]] = b[Q[i]];

    b[P[i]] = a[Q[i]];
    a[P[i]] = some_calculation(CALC_SIZE);
}


__kernel void loop_kernel_1
(
__global float *a,
__global float *b,
__global int *Q,
__global int *P,
__global IndexNode *index_node,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
	// if

    int i = index_node[get_global_id(0)].index;
    	
	a[P[i]] = b[Q[i]];

    a[Q[i]] = b[P[i]] * b[Q[i]];
    a[P[i]] = some_calculation(CALC_SIZE);
}



__kernel void evaluate_condition_kernel
(
__global float *c,
__global IndexNode *ind_cond_val,
__const int LOOP_SIZE
)
{
    int i = get_global_id(0);
    ind_cond_val[i].condVal = select(0, 1, c[i] > 0);
    ind_cond_val[i].index = i;
}



__kernel void check_loopkernel_0_writeon_a
(
__global int *buffer,
__global int *P,
__global IndexNode *ind_cond_val,
__global int *raceFlag
)
{
    int i = ind_cond_val[get_global_id(0)].index;

    int writeTimes = atomic_inc(&buffer[P[i]]);

    if (writeTimes > 0) {
        *raceFlag = 1;
    }
    
}


__kernel void check_loopkernel_0_readon_a
(
__global int *buffer,
__global int *Q,
__global IndexNode *ind_cond_val,
__global int *raceFlag
)
{
    int i = ind_cond_val[get_global_id(0)].index;

    if (buffer[Q[i]] > 0) {
        *raceFlag = 1;
    }
}


// kernel 0, as for shared array b, we don't need to check, because a is identical to b

__kernel void check_loopkernel_1_writeon_a
(
__global int *buffer,
__global int *P,
__global int *Q,
__global IndexNode *ind_cond_val,
__global int *raceFlag
) 
{
    int i = ind_cond_val[get_global_id(0)].index;

    int writeTimes = atomic_inc(&buffer[P[i]]);

    if (writeTimes > 0) {
        raceFlag = 1;
    }

    writeTimes = atomic_inc(&buffer[Q[i]]);

    if (writeTimes > 0) {
        raceFlag = 1;
    }

}











