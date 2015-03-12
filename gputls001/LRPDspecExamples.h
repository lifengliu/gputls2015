/*
 * LRPDspecExamples.h
 *
 *  Created on: Mar 10, 2015
 *      Author: hyliu
 */

#ifndef GPUTLS001_LRPDSPECEXAMPLES_H_
#define GPUTLS001_LRPDSPECEXAMPLES_H_

/*


	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}

*/

#include "TraceSet.h"
#include <CL/opencl.h>

class LRPDspecExamples {

public:
	LRPDspecExamples(int LOOP_SIZE, int CALC_SIZE, int ARRAY_SIZE, cl_device_id device);
	virtual ~LRPDspecExamples();

	void initArrayValues();

	void sequentialExecute();

	float someCalculation();

	void parallelExecute();

private:
	float *host_a, *host_b, *host_c, *host_d; //shared array: a, b, so read and write to a and b need to be speculative
	int *host_P, *host_Q, *host_T;  // read-only arrays

	int LOOP_SIZE, CALC_SIZE, ARRAY_SIZE;

	// since a and b are shared arrays
	TraceSet *host_read_trace_a, *host_write_trace_a;
	TraceSet *host_read_trace_b, *host_write_trace_b;


	//device resources
	cl_device_id use_device;
	cl_context context;
	cl_command_queue command_queue;
	cl_program program;
	cl_kernel loopKernel;

	//device memories
	cl_mem device_a, device_b, device_c, device_d;
	cl_mem device_P, device_Q, device_T;
	cl_mem device_read_trace_a, device_write_trace_a; //a
	cl_mem device_read_trace_b, device_write_trace_b; //b



	void assign_host_memory();
	void release_host_memory();

	void initializeDevices();
	void assign_device_memory();
	void destroy_device_memory();
	void release_other_resources();



};



#endif /* GPUTLS001_LRPDSPECEXAMPLES_H_ */




