/*
 * BeforeCheckingExamples.h
 *
 *  Created on: Mar 9, 2015
 *      Author: hyliu
 */

#ifndef GPUTLS001_BEFORECHECKINGEXAMPLES_H_
#define GPUTLS001_BEFORECHECKINGEXAMPLES_H_


#include <CL/opencl.h>
#include "utils.h"
/*
Example 1:

for int i = 1 to 100000 do {
    a[P[i]] = b[Q[i]] + c[Q[i]];
    b[T[i]] = f(i);
    d[i] = g(i);

    some_calculation();

}


In this example, a is a shared array
b is shared array
c P Q T d is not shared array

 */




class BeforeCheckingExamples {

public:
	BeforeCheckingExamples(int LOOP_SIZE, int CALC_SIZE, cl_device_id device);

	virtual ~BeforeCheckingExamples();

	void example1();

	void sequentialExecute();

	void parallelExecute();

	void initArrayValues();

	float someCalculation();

	bool parallelCheck();

private:
	float *host_a, *host_b, *host_c, *host_d;
	int *host_P, *host_Q, *host_T;
	int *host_buffer;
	int host_raceFlag;

	int LOOP_SIZE, CALC_SIZE;


	cl_device_id use_device;
	cl_context context;
	cl_command_queue command_queue;
	cl_program program;

	cl_kernel loopKernel, markwritePKernel, markwriteTKernel, markReadQKernel;

	cl_mem device_a, device_b, device_c, device_d;
	cl_mem device_P, device_Q, device_T;

	cl_mem device_buffer;
	cl_mem device_raceFlag;

	void assign_host_memory();
	void initializeDevices();
	void assign_device_memory();
	void destroy_device_memory();
	void release_other_resources();

};




#endif /* GPUTLS001_BEFORECHECKINGEXAMPLES_H_ */





