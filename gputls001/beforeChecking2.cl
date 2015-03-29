/*



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
__global int *Q,
__global int *P,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
    int i = get_global_id(0);
    
    a[P[i]] = b[Q[i]];

    if (b[i] > 0) {
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
__global int *index,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
    int i = index[get_global_id(0)]; 
    // else
    b[P[i]] = a[Q[i]];
    a[P[i]] = some_calculation(CALC_SIZE);
}


__kernel void loop_kernel_1
(
__global float *a,
__global float *b,
__global int *Q,
__global int *P,
__global int *index,
__const int LOOP_SIZE,
__const int CALC_SIZE
)
{
    int i = index[get_global_id(0)];
    // if
    a[Q[i]] = b[P[i]] * b[Q[i]];
    a[P[i]] = some_calculation(CALC_SIZE);
}


__kernel void evaluate_condition_kernel
(
__global float *c,
__global int *condition_val,
__const int LOOP_SIZE
)
{
    int i = get_global_id(0);
    condition_val[i] = select(0, 1, c[i] > 0);
}








