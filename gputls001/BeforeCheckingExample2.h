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





struct IndexNode;

class BeforeCheckingExample2 {
public:
	BeforeCheckingExample2(int LOOP_SIZE, int CALC_SIZE, int ARRAY_SIZE, cl_device_id device);
	virtual ~BeforeCheckingExample2();

	void parallelExecute();
	void evaluateConditions();
	void initArrayValues();
	void sortBuildIndex();
	bool parallelCheck();


	bool checkBranch0();
	bool checkBranch1();

private:
	float *host_a, *host_b;
	float *host_c;
	int *host_P, *host_Q;
	int *host_buffer;

	IndexNode *host_index_node;

	int host_raceFlag;

	int LOOP_SIZE, CALC_SIZE, ARRAY_SIZE;

	cl_device_id use_device;
	cl_context context;
	cl_command_queue command_queue;
	cl_program program;

	cl_kernel loopKernelOrigin, loopKernel0, loopKernel1, evalCondKernel;
	cl_kernel check_loopkernel_0_writeon_a, check_loopkernel_0_readon_a, check_loopkernel_1_writeon_a;

	cl_mem device_a, device_b;
	cl_mem device_P, device_Q;
	cl_mem device_c;

	cl_mem device_buffer;
	cl_mem device_raceFlag;

	cl_mem device_index_node;

	void assign_host_memory();
	void initializeDevices();
	void assign_device_memory();
	void destroy_device_memory();
	void release_other_resources();

	void clear_writebuffer();

};


#endif /* GPUTLS001_BEFORECHECKINGEXAMPLE2_H_ */
