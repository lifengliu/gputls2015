/*
 * BeforeCheckingExample2.h
 *
 *  Created on: Mar 24, 2015
 *      Author: hyliu
 */

#ifndef GPUTLS001_BEFORECHECKINGEXAMPLE2_H_
#define GPUTLS001_BEFORECHECKINGEXAMPLE2_H_

#include <CL/opencl.h>

/*

suppose branch conditions are immutable


check all branch conditions



for int i = 1 to 100000 do {
    a[P[i]] = b[Q[i]];

    if (b[i] > 0) { // in condition, it is considered as read
        a[Q[i]] = f(i);
    } else {
        b[P[i]] = a[Q[i]];
    }

    some_calculation();
}
 */



/*




 */



class BeforeCheckingExample2 {
public:
	BeforeCheckingExample2(int LOOP_SIZE, int CALC_SIZE, int ARRAY_SIZE, cl_device_id device);
	virtual ~BeforeCheckingExample2();

	void parallelExecute();
	void evaluateConditions();
	void initArrayValues();

private:
	float *host_a, *host_b;
	float *host_c;
	int *host_P, *host_Q;

	int *host_buffer, *host_condition_val;

	int host_raceFlag;

	int LOOP_SIZE, CALC_SIZE, ARRAY_SIZE;


	cl_device_id use_device;
	cl_context context;
	cl_command_queue command_queue;
	cl_program program;

	cl_kernel loopKernelOrigin, loopKernel0, loopKernel1, evalCondKernel;

	cl_mem device_a, device_b;
	cl_mem device_P, device_Q;
	cl_mem device_c;

	cl_mem device_buffer;
	cl_mem device_raceFlag;
	cl_mem device_condition_val;

	void assign_host_memory();
	void initializeDevices();
	void assign_device_memory();
	void destroy_device_memory();
	void release_other_resources();


};


#endif /* GPUTLS001_BEFORECHECKINGEXAMPLE2_H_ */
