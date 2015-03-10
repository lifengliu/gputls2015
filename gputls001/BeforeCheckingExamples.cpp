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
	assign_host_memory();
}


BeforeCheckingExamples::~BeforeCheckingExamples() {
	delete[] host_a;
	delete[] host_b;
	delete[] host_c;
	delete[] host_d;

	delete[] host_P;
	delete[] host_Q;
	delete[] host_T;
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
		host_b[i] = 0;

		host_P[i] = i;
		host_Q[i] = i / 2;
		host_T[i] = LOOP_SIZE - i;

	}

}

void BeforeCheckingExamples::assign_host_memory() {

	host_a = new float[LOOP_SIZE];
	host_b = new float[LOOP_SIZE];
    host_c = new float[LOOP_SIZE];
    host_d = new float[LOOP_SIZE];

    host_P = new int[LOOP_SIZE];
    host_Q = new int[LOOP_SIZE];
    host_T = new int[LOOP_SIZE];

}




bool BeforeCheckingExamples::parallelCheck() {
	cl_int clStatus;
	cl_context context = clCreateContext(NULL, 1, &use_device, NULL, NULL, &clStatus);
	printf("%d\n", clStatus);
	cl_command_queue commandQueue = clCreateCommandQueue(context, use_device, 0, &clStatus);
	printf("%d\n", clStatus);

	int sourceSize;
	char *clSourceCode = gputls::loadFile("gputls001/beforeChecking.cl", &sourceSize);
	cl_program program = clCreateProgramWithSource(context, 1, (const char **) &clSourceCode, NULL, &clStatus);

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
		return false;
	}


	/*cl_device_id device = gputls::getOneGPUDevice(1);    // 0 is APU; 1 is R9 290X
	//printfunc::display_device(device);

	cl_int clStatus;
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &clStatus);
	cl_command_queue command_queue = clCreateCommandQueue(context, device, 0, &clStatus);

	int sourceSize;
	char *clSourceCode = gputls::loadFile("gputls001/gputls.cl", &sourceSize);

	cl_program program = clCreateProgramWithSource(context, 1, (const char **) &clSourceCode, NULL, &clStatus);
	clStatus = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	if (clStatus != CL_SUCCESS) {
		printf("clStatus = %d\n", clStatus);
		puts("build Program Error");
		size_t len;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &len);
		char *log = new char[len];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len, log, NULL);
		printf("%s\n", log);
		return -1;
	}
*/


	return true;

}











