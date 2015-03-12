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

	bool dependencyChecking();




private:
	float *host_a, *host_b, *host_c, *host_d; //shared array: a, b, so read and write to a and b need to be speculative
	int *host_P, *host_Q, *host_T;  // read-only arrays

	int LOOP_SIZE, CALC_SIZE, ARRAY_SIZE;

	int *host_read_to;
	int *host_write_to;
	int *host_write_count;

	// since a and b are shared arrays
	TraceSet<5> *host_read_trace_a, *host_write_trace_a;
	TraceSet<5> *host_read_trace_b, *host_write_trace_b;


	//device resources
	cl_device_id use_device;
	cl_context context;
	cl_command_queue command_queue;
	cl_program program;
	cl_kernel loopKernel;
	cl_kernel dc1Kernel, dc2reduceKernel, dc3Kernel;

	//device memories
	cl_mem device_a, device_b, device_c, device_d;
	cl_mem device_P, device_Q, device_T;
	cl_mem device_read_trace_a, device_write_trace_a; //a
	cl_mem device_read_trace_b, device_write_trace_b; //b

	//for dc
	cl_mem device_read_to;
	cl_mem device_write_to;
	cl_mem device_write_count;




	void assign_host_memory();
	void release_host_memory();

	void initializeDevices();
	void assign_device_memory();
	void destroy_device_memory();
	void release_other_resources();

	void dc_phase1(cl_mem &readTrace, cl_mem &writeTrace);
	bool dc_phase2();
	bool dc_phase3();

	int dc_reduce(cl_mem &reduced_array, int length);


};



#endif /* GPUTLS001_LRPDSPECEXAMPLES_H_ */




