
typedef uint2 data_t;
#define getKey(a) ((a).x)
#define getValue(a) ((a).y)
#define makeData(k,v) ((uint2)((k),(v)))


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



__kernel void loop_task_kernel(__global int *P, __global int *Q, __global int *a, __global int *c, __const int N, __const int size1, __const int size2)
{
    int b0 = get_group_id(0);
    int t0 = get_local_id(0);

    #define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576)
      if (N >= t0 + c0 + 1) {
        if ( (c[t0 + c0] & 1) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 2) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 4) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 8) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 16) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

	  }
}



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
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576) {
      //if (N >= t0 + c0 + 1) {
        
        if ( (c[t0 + c0] & 1) > 0) {   
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 2) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 4) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 8) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

		if ((c[t0 + c0] & 16) > 0) {
			a[Q[t0 + c0]] += comp1(size1) + a[P[t0 + c0]];
        } else {
			a[Q[t0 + c0]] += comp2(size2);
        }

	  //}
	}

}


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
    for (int c0 = 32 * b0; c0 < N; c0 += 1048576) {
      //if (N >= t0 + c0 + 1) {
		    int i = get_global_id(0);

			int bv = ( (c[t0 + c0] & 1) > 0 ? 1 : 0);
			bv |= ( (c[t0 + c0] & 2) > 0 ? 2 : 0);
			bv |= ( (c[t0 + c0] & 4) > 0 ? 4 : 0);
			bv |= ( (c[t0 + c0] & 8) > 0 ? 8 : 0);
			bv |= ( (c[t0 + c0] & 16) > 0 ? 16 : 0);

			index_node[i] = makeData(bv, tid);
	  //}
	
	}

}





