
typedef uint2 data_t;
#define getKey(a) ((a).x)
#define getValue(a) ((a).y)
#define makeData(k,v) ((uint2)((k),(v)))

typedef struct IndexNode {
    int index;
    int condVal;
} IndexNode;


float some_calculation(int iteration, int CALC_SIZE) {
	/*float res = 0.5f;
	
	for (int i = 0; i < CALC_SIZE; i++) {
		res = sin(res);
	}
	*/
	
	float res = 0.0f;

	for (int p = 1; p <= CALC_SIZE; p++) {
		res += (p + iteration) % 5;
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
        a[Q[i]] = b[P[i]] * b[Q[i]] + some_calculation(CALC_SIZE, i);
    } else {
        b[P[i]] = a[Q[i]];
    }

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

    int tid = get_global_id(0);
    //printf("tid = %d\n", tid);
    int i = index_node[tid].index;
    	
	a[P[i]] = b[Q[i]];
	a[Q[i]] = b[P[i]] * b[Q[i]] + some_calculation(CALC_SIZE, i);

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
    int tid = get_global_id(0);
    int i = ind_cond_val[tid].index;

    int writeTimes = atomic_inc(&buffer[P[i]]);

    if (writeTimes > 0) {
        raceFlag = 1;
    }

    writeTimes = atomic_inc(&buffer[Q[i]]);

    if (writeTimes > 0) {
        raceFlag = 1;
    }

}



/*


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



// --------------------------------------------------------------------------------------------------------------------

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




// -----------------------------------------------------------------------------------------

__kernel void dc_write_on_a(
__global int *P, 
__global int *Q, 
__global int *a, 
__global int *c, 
__const int N, 
__const int size1, 
__const int size2, 
__global int *raceFlag,
__global int *buffer
)
{
    int b0 = get_group_id(0);
    int t0 = get_local_id(0);

	int writeTimes = 0;

    #define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576)
      if (N >= t0 + c0 + 1) {
			if (c[t0+c0] > 0) {
				writeTimes = atomic_inc(&buffer[Q[t0 + c0]]);
				raceFlag[0] |= writeTimes > 0;
			} else {
				writeTimes = atomic_inc(&buffer[Q[t0 + c0]]);
				raceFlag[0] |= writeTimes > 0;
			}
	  }
      
}


__kernel void dc_read_on_a(
__global int *P, 
__global int *Q, 
__global int *a, 
__global int *c, 
__const int N, 
__const int size1, 
__const int size2, 
__global int *raceFlag,
__global int *buffer
)
{
    int b0 = get_group_id(0);
    int t0 = get_local_id(0);

    #define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576)
      if (N >= t0 + c0 + 1) {
			if (c[t0 + c0] > 0) {
			    raceFlag[0] |= buffer[P[t0 + c0]] > 0 ;
			} else {
			}
	  }
}




// ---------------------------------------------------------------------------------------------------


__kernel void loop_task_kernel
(
__global int *P, 
__global int *Q, 
__global int *a, 
__global int *c, 
__const int N, 
__const int size1, 
__const int size2
)
{
    int b0 = get_group_id(0);
    int t0 = get_local_id(0);

    #define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576)
      if (N >= t0 + c0 + 1)
        if (c[t0 + c0] > 0) {
			a[Q[t0 + c0]] = comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] = comp2(size2);
        }
}



// ---------------------------------------evaluate    -------------------------------------------------------------



__kernel void branch_evaluate_task_kernel
(
__global int *P, 
__global int *Q, 
__global int *a, 
__global int *c, 
__const int N, 
__const int size1, 
__const int size2,
__global data_t *index_node
)
{
    int b0 = get_group_id(0);
    int t0 = get_local_id(0);
	int tid = get_global_id(0);

    #define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576)
      if (N >= t0 + c0 + 1) {
		    int i = get_global_id(0);
			index_node[i] = makeData(select(0, 1, c[t0 + c0] > 0), tid);
			//setKey(index_node[i], select(0, 1, c[t0 + c0] > 0));
			//setValue(index_node[i], tid);
	  }
}


// ---------------------------------------------- after remapped -------------------------------------------------------------------------------




__kernel void loop_task_kernel_remapped(
__global int *P, 
__global int *Q, 
__global int *a, 
__global int *c, 
__const int N, 
__const int size1, 
__const int size2,
__global data_t *index_node
)
{
    int tid = getValue(index_node[get_global_id(0)]);
    //globalWorkSize = localWorkSize * numberOfGroups; 

	// tid  = b0 * workgroupsize + t0;
	
	int b0 = tid / get_local_size(0);
	int t0 = tid % get_local_size(0);
	
	//int b0 = index_node[tid].groupid;  
	//reference redirection
	//int t0 = index_node[tid].localid;

    //int b0 = get_group_id(0);
    //int t0 = get_local_id(0);

    #define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576)
      if (N >= t0 + c0 + 1)
        if (c[t0 + c0] > 0) {
			a[Q[t0 + c0]] = comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] = comp2(size2);
        }
}



