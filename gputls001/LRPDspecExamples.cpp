/*
 * LRPDspecExamples.cpp
 *
 *  Created on: Mar 10, 2015
 *      Author: hyliu
 */

#include "LRPDspecExamples.h"

#include "TraceSet.h"

#include <CL/opencl.h>
#include <cstdio>
#include <cmath>
#include <sys/time.h>
#include "utils.h"

/*


	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}

*/


LRPDspecExamples::LRPDspecExamples(int LOOP_SIZE, int CALC_SIZE, int ARRAY_SIZE, cl_device_id device)
{
	this->LOOP_SIZE = LOOP_SIZE;
	this->CALC_SIZE = CALC_SIZE;
	this->ARRAY_SIZE = ARRAY_SIZE;
	this->use_device = device;

	initializeDevices();
	assign_host_memory();

	initArrayValues();
	assign_device_memory();

}




LRPDspecExamples::~LRPDspecExamples() {
	release_host_memory();
	destroy_device_memory();
	release_other_resources();
}


void LRPDspecExamples::assign_host_memory() {
	//float *host_a, *host_b, *host_c, *host_d; //shared array: a, b, so read and write to a and b need to be speculative
	//int *host_P, *host_Q, *host_T;  // read-only arrays
	host_a = new float[ARRAY_SIZE];
	host_b = new float[ARRAY_SIZE];
	host_c = new float[ARRAY_SIZE];
	host_d = new float[ARRAY_SIZE];

	host_P = new int[ARRAY_SIZE];
	host_Q = new int[ARRAY_SIZE];
	host_T = new int[ARRAY_SIZE];

	//TraceSet *host_read_trace_a, *host_write_trace_a;
	//TraceSet *host_read_trace_b, *host_write_trace_b;
	host_read_trace_a = new TraceSet[LOOP_SIZE];
	host_write_trace_a = new TraceSet [LOOP_SIZE];

	host_read_trace_b = new TraceSet [LOOP_SIZE];
	host_write_trace_b = new TraceSet [LOOP_SIZE];



}


void LRPDspecExamples::release_host_memory() {
	delete[] host_a;
	delete[] host_b;
	delete[] host_c;
	delete[] host_d;

	delete[] host_P;
	delete[] host_Q;
	delete[] host_T;

	delete[] host_read_trace_a;
	delete[] host_write_trace_a;
	delete[] host_read_trace_b;
	delete[] host_write_trace_b;


}


void LRPDspecExamples::assign_device_memory() {
	cl_int clStatus;

	device_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_a, &clStatus);
	device_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_b, &clStatus);
	device_c = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_c, &clStatus);
	device_d = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_d, &clStatus);

	device_P = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_P, &clStatus);
	device_Q = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_Q, &clStatus);
	device_T = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_T, &clStatus);

	device_read_trace_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(TraceSet),
			host_read_trace_a, &clStatus);

	device_write_trace_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(TraceSet),
			host_write_trace_a, &clStatus);

	device_read_trace_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(TraceSet),
			host_read_trace_b, &clStatus);

	device_write_trace_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(TraceSet),
			host_write_trace_b, &clStatus);

	printf("device memory assign %d\n", clStatus);


}


void LRPDspecExamples::destroy_device_memory() {
	clReleaseMemObject(device_a);
	clReleaseMemObject(device_b);
	clReleaseMemObject(device_c);
	clReleaseMemObject(device_d);
	clReleaseMemObject(device_P);
	clReleaseMemObject(device_Q);
	clReleaseMemObject(device_T);

	clReleaseMemObject(device_read_trace_a);
	clReleaseMemObject(device_write_trace_a);
	clReleaseMemObject(device_read_trace_b);
	clReleaseMemObject(device_write_trace_b);

}

void LRPDspecExamples::initializeDevices() {
	cl_int clStatus;
	context = clCreateContext(NULL, 1, &use_device, NULL, NULL, &clStatus);
	printf("create context clStatus = %d\n", clStatus);
	command_queue = clCreateCommandQueue(context, use_device, 0, &clStatus);
	printf("command queue create %d\n", clStatus);

	int sourceSize;
	char *clSourceCode = gputls::loadFile("gputls001/lrpd_spec.cl", &sourceSize);
	program = clCreateProgramWithSource(context, 1, (const char **) &clSourceCode, NULL, &clStatus);

	printf("program %d\n", clStatus);

	clStatus = clBuildProgram(program, 1, &use_device, NULL, NULL, NULL);

	if (clStatus != CL_SUCCESS) {
		printf("clStatus = %d\n", clStatus);
		puts("build Program Error");
		size_t len;
		clGetProgramBuildInfo(program, use_device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &len);
		char *log = new char[len];
		clGetProgramBuildInfo(program, use_device, CL_PROGRAM_BUILD_LOG, len, log, NULL);
		printf("%s\n", log);
	}

	loopKernel = clCreateKernel(program, "loop_kernel", &clStatus);
	printf("loop kernel %d\n", clStatus);

}




void LRPDspecExamples::initArrayValues() {

	memset(host_a, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_b, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_c, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_d, 0, sizeof(int) * ARRAY_SIZE);

	memset(host_read_trace_a, 0, sizeof(TraceSet) * LOOP_SIZE);
	memset(host_read_trace_b, 0, sizeof(TraceSet) * LOOP_SIZE);
	memset(host_write_trace_a, 0, sizeof(TraceSet) * LOOP_SIZE);
	memset(host_write_trace_b, 0, sizeof(TraceSet) * LOOP_SIZE);

	for (int i = 0; i < LOOP_SIZE; i++) {
		host_P[i] = i*2;   // 0 2 4 6 8
		host_Q[i] = i*2;   // 0 2 4 6 8
		host_T[i] = i*2+1; // 1 3 5 7 9
	}


}


void LRPDspecExamples::release_other_resources() {
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	clReleaseKernel(loopKernel);
	clReleaseProgram(program);
}


void LRPDspecExamples::sequentialExecute() {
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}

	gettimeofday(&tv2, NULL);
	double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	printf("sequential execute use time = %.2f\n", used_time);
}

float LRPDspecExamples::someCalculation() {
	float res = 0.5;
	for (int i = 0; i < CALC_SIZE; i++) {
		res = sin(res);
	}
	return res;
}


void LRPDspecExamples::parallelExecute() {
	puts("parallel execute");
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	cl_int clStatus = -1;

	cl_uint p = 0;
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_a);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_b);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_c);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_d);

	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_Q);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_P);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_T);

	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_read_trace_a);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_write_trace_a);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_read_trace_b);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_write_trace_b);

	clSetKernelArg(loopKernel, p++, sizeof(cl_int), &LOOP_SIZE);
	clSetKernelArg(loopKernel, p++, sizeof(cl_int), &CALC_SIZE);


	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, loopKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	printf("enqueue nd range kernel clStatus = %d\n", clStatus);

	clStatus = clEnqueueReadBuffer(command_queue, device_b, CL_TRUE, 0, ARRAY_SIZE * sizeof(float), host_b, 0, NULL, NULL);

	printf("clReadBuffer clStatus = %d\n", clStatus);
	clFlush(command_queue);
	clFinish(command_queue);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		printf("%.2f  ", host_b[i]);
	}

	/*puts("");
	for (int i = 0; i < ARRAY_SIZE; i++) {
		printf("%d  ", host_T[i]);
	}*/

	gettimeofday(&tv2, NULL);
	double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	printf("parallel execute time = %.2f\n", used_time);



}






