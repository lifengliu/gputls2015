/*
 * BeforeCheckingExamples.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: hyliu
 */

#include "BeforeCheckingExamples.h"
#include <cmath>
#include <CL/opencl.h>
#include <cstdio>
#include <sys/time.h>

//#include "utils.h"

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


BeforeCheckingExamples::BeforeCheckingExamples(int LOOP_SIZE, int CALC_SIZE, cl_device_id device) {
	this->LOOP_SIZE = LOOP_SIZE;
	this->CALC_SIZE = CALC_SIZE;
	this->use_device = device;

	initializeDevices();
	assign_host_memory();

	initArrayValues();
	assign_device_memory(); // after initialize host values
}


BeforeCheckingExamples::~BeforeCheckingExamples() {
	delete[] host_a;
	delete[] host_b;
	delete[] host_c;
	delete[] host_d;

	delete[] host_P;
	delete[] host_Q;
	delete[] host_T;

	destroy_device_memory();
	release_other_resources();
}


void BeforeCheckingExamples::example1() {

}


float BeforeCheckingExamples::someCalculation() {
	float res = 0.5;
	for (int i = 0; i < CALC_SIZE; i++) {
		res = sin(res);
	}
	return res;
}


void BeforeCheckingExamples::sequentialExecute() {
	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}
}

void BeforeCheckingExamples::initArrayValues() {
	// this init values contain no data race

	for (int i = 0; i < LOOP_SIZE; i++) {
		host_a[i] = 0;
		host_c[i] = i % 20;
		host_d[i] = 0;
		host_b[i] = 50;

		host_P[i] = i;
		host_Q[i] = i;
		host_T[i] = LOOP_SIZE -1 - i;

	}

    memset(host_buffer, 0, sizeof(int) * LOOP_SIZE);
}


void BeforeCheckingExamples::assign_host_memory() {

	host_a = new float[LOOP_SIZE];
	host_b = new float[LOOP_SIZE];
    host_c = new float[LOOP_SIZE];
    host_d = new float[LOOP_SIZE];

    host_P = new int[LOOP_SIZE];
    host_Q = new int[LOOP_SIZE];
    host_T = new int[LOOP_SIZE];

    host_buffer = new int[LOOP_SIZE];

}


void BeforeCheckingExamples::initializeDevices() {
	cl_int clStatus;
	context = clCreateContext(NULL, 1, &use_device, NULL, NULL, &clStatus);
	printf("create context clStatus = %d\n", clStatus);
	command_queue = clCreateCommandQueue(context, use_device, 0, &clStatus);
	printf("%d\n", clStatus);

	int sourceSize;
	char *clSourceCode = gputls::loadFile("gputls001/beforeChecking.cl", &sourceSize);
	program = clCreateProgramWithSource(context, 1, (const char **) &clSourceCode, NULL, &clStatus);

	printf("%d\n", clStatus);

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
	printf("%d\n", clStatus);

	markwritePKernel = clCreateKernel(program, "markwriteP", &clStatus);
	printf("%d\n", clStatus);

	markwriteTKernel = clCreateKernel(program, "markwriteT", &clStatus);
	printf("%d\n", clStatus);

	markReadQKernel = clCreateKernel(program, "markReadQ", &clStatus);
	printf("%d\n", clStatus);


}


void BeforeCheckingExamples::assign_device_memory() {
	cl_int clStatus;
	device_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(float), host_a, &clStatus);
	device_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(float), host_b, &clStatus);
	device_c = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(float), host_c, &clStatus);
	device_d = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(float), host_d, &clStatus);

	device_P = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(int), host_P, &clStatus);
	device_Q = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(int), host_Q, &clStatus);
	device_T = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(int), host_T, &clStatus);

	device_raceFlag = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), &host_raceFlag, &clStatus);
}

void BeforeCheckingExamples::destroy_device_memory() {
	clReleaseMemObject(device_a);
	clReleaseMemObject(device_b);
	clReleaseMemObject(device_c);
	clReleaseMemObject(device_d);

	clReleaseMemObject(device_P);
	clReleaseMemObject(device_Q);
	clReleaseMemObject(device_T);

	clReleaseMemObject(device_raceFlag);

}

bool BeforeCheckingExamples::parallelCheck() {
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);


	return true;

}



void BeforeCheckingExamples::release_other_resources() {
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	clReleaseKernel(loopKernel);
	clReleaseKernel(markwritePKernel);
	clReleaseKernel(markwriteTKernel);
	clReleaseKernel(markReadQKernel);
	clReleaseProgram(program);

}



void BeforeCheckingExamples::parallelExecute() {
	/*__kernel void loop_kernel
	(
	__global float *a,
	__global float *b,
	__global float *c,
	__global float *d,
	__global int *Q,
	__global int *P,
	__global int *T,
	__const int LOOP_SIZE,
	__const int CALC_SIZE
	)
	{
	    size_t tid = get_global_id(0);
	    a[P[tid]] = b[Q[tid]] + c[Q[tid]];
	    b[T[tid]] = 500;
	    d[tid] = someCalculation(CALC_SIZE);
	}*/

	cl_int clStatus = -1;
	cl_uint p = 0;
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_a);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_b);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_c);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_d);

	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_Q);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_P);
	clSetKernelArg(loopKernel, p++, sizeof(cl_mem), &device_T);

	clSetKernelArg(loopKernel, p++, sizeof(cl_int), &LOOP_SIZE);
	clSetKernelArg(loopKernel, p++, sizeof(cl_int), &CALC_SIZE);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, loopKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	printf("enqueue nd range kernel clStatus = %d\n", clStatus);

	clStatus = clEnqueueReadBuffer(command_queue, device_b, CL_TRUE, 0, LOOP_SIZE * sizeof(float), host_b, 0, NULL, NULL);

	clStatus = clEnqueueReadBuffer(command_queue, device_T, CL_TRUE, 0, LOOP_SIZE * sizeof(int), host_T, 0, NULL, NULL);


	printf("clReadBuffer clStatus = %d\n", clStatus);
	clFlush(command_queue);
	clFinish(command_queue);

	for (int i = 0; i < LOOP_SIZE; i++) {
		printf("%.2f  ", host_b[i]);
	}

	puts("");
	for (int i = 0; i < LOOP_SIZE; i++) {
		printf("%d  ", host_T[i]);
	}





	//clEnqueueReadBuffer(command_queue, device_A, CL_TRUE, 0, NUM_VALUES * sizeof(float), host_A, 0, NULL, NULL);


}





