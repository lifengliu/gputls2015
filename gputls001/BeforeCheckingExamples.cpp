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


const bool DEBUG = false;

BeforeCheckingExamples::BeforeCheckingExamples(int LOOP_SIZE, int CALC_SIZE, int ARRAY_SIZE, cl_device_id device) {
	this->LOOP_SIZE = LOOP_SIZE;
	this->CALC_SIZE = CALC_SIZE;
	this->ARRAY_SIZE = ARRAY_SIZE;
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
	//struct timeval tv1, tv2;
	//gettimeofday(&tv1, NULL);

	initArrayValues();

	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}

	//gettimeofday(&tv2, NULL);
	//double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	//printf("sequential execute use time = %.2f\n", used_time);
}

void BeforeCheckingExamples::initArrayValues() {
	// this init values contain no data race

	memset(host_a, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_b, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_c, 0, sizeof(int) * ARRAY_SIZE);
	memset(host_d, 0, sizeof(int) * ARRAY_SIZE);


	for (int i = 0; i < LOOP_SIZE; i++) {
		host_P[i] = i*2;   // 0 2 4 6 8
		host_Q[i] = i*2;   // 0 2 4 6 8
		host_T[i] = i*2+1; // 1 3 5 7 9
	}

    memset(host_buffer, 0, sizeof(int) * ARRAY_SIZE);
}


void BeforeCheckingExamples::assign_host_memory() {

	host_a = new float[ARRAY_SIZE];
	host_b = new float[ARRAY_SIZE];
    host_c = new float[ARRAY_SIZE];
    host_d = new float[ARRAY_SIZE];

    host_P = new int[ARRAY_SIZE];
    host_Q = new int[ARRAY_SIZE];
    host_T = new int[ARRAY_SIZE];

    host_buffer = new int[ARRAY_SIZE];

}


void BeforeCheckingExamples::initializeDevices() {
	cl_int clStatus;
	context = clCreateContext(NULL, 1, &use_device, NULL, NULL, &clStatus);

	if (DEBUG) {
		printf("create context clStatus = %d\n", clStatus);
	}

	command_queue = clCreateCommandQueue(context, use_device, 0, &clStatus);

	if (DEBUG) {
		printf("%d\n", clStatus);
	}

	int sourceSize;
	char *clSourceCode = gputls::loadFile("gputls001/beforeChecking.cl", &sourceSize);
	program = clCreateProgramWithSource(context, 1, (const char **) &clSourceCode, NULL, &clStatus);

	if (DEBUG) {
		printf("%d\n", clStatus);
	}

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
	//printf("%d\n", clStatus);

	markwritePKernel = clCreateKernel(program, "markwriteP", &clStatus);
	//printf("%d\n", clStatus);

	markwriteTKernel = clCreateKernel(program, "markwriteT", &clStatus);
	//printf("%d\n", clStatus);

	markReadQKernel = clCreateKernel(program, "markReadQ", &clStatus);
	//printf("%d\n", clStatus);


}


void BeforeCheckingExamples::assign_device_memory() {
	cl_int clStatus;
	device_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_a, &clStatus);
	device_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_b, &clStatus);
	device_c = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_c, &clStatus);
	device_d = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_d, &clStatus);

	device_P = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_P, &clStatus);
	device_Q = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_Q, &clStatus);
	device_T = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_T, &clStatus);

	host_raceFlag = 0;
	device_raceFlag = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &host_raceFlag, &clStatus);
	device_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_buffer, &clStatus);
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
	clReleaseMemObject(device_buffer);
}

bool BeforeCheckingExamples::check_write_P() {

	cl_int clStatus = -1;
	cl_uint p = 0;

	clSetKernelArg(markwritePKernel, p++, sizeof(cl_mem), &device_buffer);
	clSetKernelArg(markwritePKernel, p++, sizeof(cl_mem), &device_P);
	clSetKernelArg(markwritePKernel, p++, sizeof(cl_mem), &device_raceFlag);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	//printf("enqueue kernel %d\n", clStatus);

	clStatus = clEnqueueNDRangeKernel(command_queue, markwritePKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	//printf("enqueue kernel %d\n", clStatus);

	int flag = 0;
	clStatus = clEnqueueReadBuffer(command_queue, device_raceFlag, CL_TRUE, 0, sizeof(int), &flag, 0, NULL, NULL);

	if (DEBUG) {
		for (int i = 0; i < ARRAY_SIZE; i++) {
			printf("%d ", host_buffer[i]);
		}
		clStatus = clEnqueueReadBuffer(command_queue, device_buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, host_buffer, 0, NULL, NULL);
	}

	if (flag == 0) {
		return false;
	} else {
		return true;
	}
}



bool BeforeCheckingExamples::check_read_Q() {
	cl_int clStatus = -1;
	cl_uint p = 0;

	clSetKernelArg(markReadQKernel, p++, sizeof(cl_mem), &device_buffer);
	clSetKernelArg(markReadQKernel, p++, sizeof(cl_mem), &device_Q);
	clSetKernelArg(markReadQKernel, p++, sizeof(cl_mem), &device_raceFlag);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, markReadQKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("enqueue kernel %d\n", clStatus);
	}

	int flag = 0;
	clStatus = clEnqueueReadBuffer(command_queue, device_raceFlag, CL_TRUE, 0, sizeof(int), &flag, 0, NULL, NULL);

	if (DEBUG) {
		printf("flag = %d\n", flag);

		clStatus = clEnqueueReadBuffer(command_queue, device_buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, host_buffer, 0, NULL, NULL);

		for (int i = 0; i < ARRAY_SIZE; i++) {
			printf("%d ", host_buffer[i]);
		}

		puts("");
	}


	if (flag == 0) {
		return false;
	} else {
		return true;
	}
}

bool BeforeCheckingExamples::check_write_T() {
	cl_int clStatus = -1;
	cl_uint p = 0;

	clSetKernelArg(markwriteTKernel, p++, sizeof(cl_mem), &device_buffer);
	clSetKernelArg(markwriteTKernel, p++, sizeof(cl_mem), &device_T);
	clSetKernelArg(markwriteTKernel, p++, sizeof(cl_mem), &device_raceFlag);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	//printf("enqueue kernel %d\n", clStatus);

	clStatus = clEnqueueNDRangeKernel(command_queue, markwriteTKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("enqueue kernel %d\n", clStatus);
	}

	int flag = 0;
	clStatus = clEnqueueReadBuffer(command_queue, device_raceFlag, CL_TRUE, 0, sizeof(int), &flag, 0, NULL, NULL);

	if (DEBUG) {
		clStatus = clEnqueueReadBuffer(command_queue, device_buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, host_buffer, 0, NULL, NULL);

		printf("flag = %d\n", flag);

		for (int i = 0; i < ARRAY_SIZE; i++) {
			printf("%d ", host_buffer[i]);
		}

		puts("");
	}


	if (flag == 0) {
		return false;
	} else {
		return true;
	}

}



bool BeforeCheckingExamples::parallelCheck() {
	//struct timeval tv1, tv2;
	//gettimeofday(&tv1, NULL);

	cl_int clStatus;
	bool conflict = check_write_P();

	if (!conflict) {
		// fill Buffer to zero
		clReleaseMemObject(device_buffer);
		memset(host_buffer, 0, sizeof(int) * ARRAY_SIZE);
		device_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_buffer, &clStatus);

		conflict = check_write_T();

		if (!conflict) {
			conflict = check_read_Q();

		}

	}

	//gettimeofday(&tv2, NULL);
	//double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	//printf("check used time = %.2f\n", used_time);

	return conflict != 0;;
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

	if (DEBUG) {
		puts("parallel execute");
	}

	//struct timeval tv1, tv2;
	//gettimeofday(&tv1, NULL);

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
	//printf("enqueue nd range kernel clStatus = %d\n", clStatus);

	clStatus = clEnqueueReadBuffer(command_queue, device_b, CL_TRUE, 0, ARRAY_SIZE * sizeof(float), host_b, 0, NULL, NULL);

	clStatus = clEnqueueReadBuffer(command_queue, device_T, CL_TRUE, 0, ARRAY_SIZE * sizeof(int), host_T, 0, NULL, NULL);


	//printf("clReadBuffer clStatus = %d\n", clStatus);
	clFlush(command_queue);
	clFinish(command_queue);

	/*for (int i = 0; i < ARRAY_SIZE; i++) {
		printf("%.2f  ", host_b[i]);
	}

	puts("");
	for (int i = 0; i < ARRAY_SIZE; i++) {
		printf("%d  ", host_T[i]);
	}*/

	//gettimeofday(&tv2, NULL);
	//double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	//printf("parallel execute time = %.2f\n", used_time);

	//clEnqueueReadBuffer(command_queue, device_A, CL_TRUE, 0, NUM_VALUES * sizeof(float), host_A, 0, NULL, NULL);


}





