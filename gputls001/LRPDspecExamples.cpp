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

	//TraceSet<5> *host_read_trace_a, *host_write_trace_a;
	//TraceSet<5> *host_read_trace_b, *host_write_trace_b;
	host_read_trace_a = new TraceSet<5>[LOOP_SIZE];
	host_write_trace_a = new TraceSet<5> [LOOP_SIZE];

	host_read_trace_b = new TraceSet<5> [LOOP_SIZE];
	host_write_trace_b = new TraceSet<5> [LOOP_SIZE];


	host_read_to = new int[ARRAY_SIZE];
	host_write_to = new int[ARRAY_SIZE];

	host_write_count = new int[LOOP_SIZE];

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

	delete[] host_read_to;
	delete[] host_write_to;
	delete[] host_write_count;

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
			LOOP_SIZE * sizeof(TraceSet<5>),
			host_read_trace_a, &clStatus);

	device_write_trace_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(TraceSet<5>),
			host_write_trace_a, &clStatus);

	device_read_trace_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(TraceSet<5>),
			host_read_trace_b, &clStatus);

	device_write_trace_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(TraceSet<5>),
			host_write_trace_b, &clStatus);

	device_read_to = clCreateBuffer(context,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			ARRAY_SIZE * sizeof(int), host_read_to, &clStatus);

	device_write_to = clCreateBuffer(context,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			ARRAY_SIZE * sizeof(int), host_write_to, &clStatus);

	device_write_count = clCreateBuffer(context,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			LOOP_SIZE * sizeof(int), host_write_count, &clStatus);


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
	dc1Kernel = clCreateKernel(program, "dc_phase1", &clStatus);
	dc2reduceKernel = clCreateKernel(program, "reduce", &clStatus);
	printf("loop kernel %d\n", clStatus);

}




void LRPDspecExamples::initArrayValues() {

	memset(host_a, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_b, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_c, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_d, 0, sizeof(int) * ARRAY_SIZE);

	memset(host_read_trace_a, 0, sizeof(TraceSet<5>) * LOOP_SIZE);
	memset(host_read_trace_b, 0, sizeof(TraceSet<5>) * LOOP_SIZE);
	memset(host_write_trace_a, 0, sizeof(TraceSet<5>) * LOOP_SIZE);
	memset(host_write_trace_b, 0, sizeof(TraceSet<5>) * LOOP_SIZE);

	memset(host_read_to, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_write_to, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_write_count, 0, sizeof(int) * LOOP_SIZE);


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

	//for (int i = 0; i < ARRAY_SIZE; i++) {
	//	printf("%.2f  ", host_b[i]);
	//}

	/*puts("");
	for (int i = 0; i < ARRAY_SIZE; i++) {
		printf("%d  ", host_T[i]);
	}*/

	gettimeofday(&tv2, NULL);
	double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	printf("parallel execute time = %.2f\n", used_time);



}



void LRPDspecExamples::dc_phase1(cl_mem &readTrace, cl_mem &writeTrace) {

	puts("dc1");

	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	cl_int clStatus = -1;

	cl_uint p = 0;
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &readTrace);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &writeTrace);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_read_to);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_write_to);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_write_count);



	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, dc1Kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	printf("enqueue nd range kernel clStatus = %d\n", clStatus);

	clStatus = clEnqueueReadBuffer(command_queue, device_write_to, CL_TRUE, 0, ARRAY_SIZE * sizeof(int), host_write_to, 0, NULL, NULL);

	printf("read write trace from gpu clStatus = %d\n", clStatus);

	clFlush(command_queue);
	clFinish(command_queue);

	//for (int i = 0; i < ARRAY_SIZE; i++) {
	//	printf("%d  ", host_write_to[i]);
	//}

	//puts("");
	gettimeofday(&tv2, NULL);
	double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	printf("dc1 time = %.2f\n", used_time);

}



bool LRPDspecExamples::dc_phase2() {
	puts("reduction");

	int array1WriteTo = dc_reduce(device_write_to, ARRAY_SIZE);
	int array1WriteCount = dc_reduce(device_write_count, LOOP_SIZE);



	return false;
}

int LRPDspecExamples::dc_reduce(cl_mem &reduced_array, int length) {
	cl_int clStatus;

	size_t localWorkSize = 128;
	size_t numWorkGroups = 64;
	size_t globalWorkSize = numWorkGroups * localWorkSize;

	cl_mem device_reducedWriteTo = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numWorkGroups * sizeof(int), NULL, &clStatus);

	clSetKernelArg(dc2reduceKernel, 0, sizeof(cl_mem), &reduced_array);
	clSetKernelArg(dc2reduceKernel, 1, sizeof(cl_int) * localWorkSize, NULL);
	clSetKernelArg(dc2reduceKernel, 2, sizeof(cl_int), &length);
	clSetKernelArg(dc2reduceKernel, 3, sizeof(cl_mem), &device_reducedWriteTo);

	clStatus = clEnqueueNDRangeKernel(command_queue, dc2reduceKernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);

	int *reduced_writeToResult = new int[numWorkGroups];
	clEnqueueReadBuffer(command_queue, device_reducedWriteTo, CL_TRUE, 0, sizeof(int) * numWorkGroups, reduced_writeToResult, 0, NULL, NULL);

	int writeToSum = 0;
	for (size_t i = 0; i < numWorkGroups; i++) {
		writeToSum += reduced_writeToResult[i];
		//printf("%d ", reduced_writeToResult[i]);
	}

	printf("%d\n", writeToSum);

	clReleaseMemObject(device_reducedWriteTo);
	delete[] reduced_writeToResult;

	return writeToSum;
}



bool LRPDspecExamples::dependencyChecking() {
	//first check a
	dc_phase1(device_read_trace_a, device_write_trace_a);
	dc_phase2();
	return false;
}




