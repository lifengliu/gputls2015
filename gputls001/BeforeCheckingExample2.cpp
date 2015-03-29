/*
 * BeforeCheckingExample2.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: hyliu
 */

#include "BeforeCheckingExample2.h"

#include <CL/opencl.h>
#include <cstdio>
#include <cmath>
#include <sys/time.h>
#include "utils.h"

const bool DEBUG = true;

BeforeCheckingExample2::BeforeCheckingExample2(int LOOP_SIZE, int CALC_SIZE, int ARRAY_SIZE, cl_device_id device) {
	this->LOOP_SIZE = LOOP_SIZE;
	this->CALC_SIZE = CALC_SIZE;
	this->ARRAY_SIZE = ARRAY_SIZE;
	this->use_device = device;
	
	initializeDevices();

	assign_host_memory();

	initArrayValues();
	assign_device_memory(); 

}

BeforeCheckingExample2::~BeforeCheckingExample2() {
	delete[] host_a;
	delete[] host_b;
	delete[] host_c;

	delete[] host_P;
	delete[] host_Q;
	delete[] host_condition_val;
	delete[] host_buffer;

	destroy_device_memory();

	release_other_resources();

}

void BeforeCheckingExample2::initializeDevices() {
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
	char *clSourceCode = gputls::loadFile("gputls001/beforeChecking2.cl", &sourceSize);
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

	loopKernelOrigin = clCreateKernel(program, "loop_kernel_origin", &clStatus);
	if (DEBUG) {
		printf("loopKernel create status = %d\n", clStatus);
	}


	loopKernel0 = clCreateKernel(program, "loop_kernel_0", &clStatus);
	if (DEBUG) {
		printf("loopKernel0 create status = %d\n", clStatus);
	}

	loopKernel1 = clCreateKernel(program, "loop_kernel_1", &clStatus);
	if (DEBUG) {
		printf("loopKernel1 create status = %d\n", clStatus);
	}

	evalCondKernel = clCreateKernel(program, "evaluate_condition_kernel", &clStatus);
	if (DEBUG) {
		printf("evalute condition kernel status = %d\n", clStatus);
	}

}



void BeforeCheckingExample2::assign_host_memory() {
	host_a = new float[ARRAY_SIZE];
	host_b = new float[ARRAY_SIZE];
	host_P = new int[ARRAY_SIZE];
	host_Q = new int[ARRAY_SIZE];
	host_c = new float[ARRAY_SIZE];
    host_buffer = new int[ARRAY_SIZE];
    host_condition_val = new int[LOOP_SIZE];
}


void BeforeCheckingExample2::assign_device_memory() {
	cl_int clStatus;

	device_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_a, &clStatus);
	device_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_b, &clStatus);
	
	device_P = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_P, &clStatus);
	device_Q = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_Q, &clStatus);

	device_c = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_c, &clStatus);

	host_raceFlag = 0;
	device_raceFlag = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &host_raceFlag, &clStatus);
	device_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_buffer, &clStatus);
	device_condition_val = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * LOOP_SIZE, host_condition_val, &clStatus);


}



void BeforeCheckingExample2::destroy_device_memory() {
	clReleaseMemObject(device_a);
	clReleaseMemObject(device_b);
	clReleaseMemObject(device_c);

	clReleaseMemObject(device_P);
	clReleaseMemObject(device_Q);
	
	clReleaseMemObject(device_raceFlag);
	clReleaseMemObject(device_buffer);
}




void BeforeCheckingExample2::release_other_resources() {
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	clReleaseKernel(loopKernel0);
	clReleaseKernel(loopKernel1);
	clReleaseKernel(loopKernelOrigin);
	clReleaseProgram(program);
}





void BeforeCheckingExample2::parallelExecute() {
	if (DEBUG) {
		puts("parallel execute");
	}

	/*cl_int clStatus = -1;
	cl_uint p = 0;

	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_a);
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_b);

	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_Q);
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_P);

	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_int), &LOOP_SIZE);
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_int), &CALC_SIZE);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, loopKernelOrigin, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	
	if (DEBUG) {
	    printf("enqueue nd range kernel clStatus = %d\n", clStatus);
	}
	
	clStatus = clEnqueueReadBuffer(command_queue, device_b, CL_TRUE, 0, ARRAY_SIZE * sizeof(float), host_b, 0, NULL, NULL);

	clStatus = clEnqueueReadBuffer(command_queue, device_T, CL_TRUE, 0, ARRAY_SIZE * sizeof(int), host_T, 0, NULL, NULL);

	if (DEBUG) {
		printf("clReadBuffer clStatus = %d\n", clStatus);
	}


	clFlush(command_queue);
	clFinish(command_queue);
	*/

}




void BeforeCheckingExample2::evaluateConditions() {
	if (DEBUG) {
		puts("evalute conditions");
	}

	cl_int clStatus ;
	int p = 0;
	clSetKernelArg(evalCondKernel, p++, sizeof(cl_mem), &device_c);
	clSetKernelArg(evalCondKernel, p++, sizeof(cl_mem), &device_condition_val);
	clSetKernelArg(evalCondKernel, p++, sizeof(cl_int), &LOOP_SIZE);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;
	
	clStatus = clEnqueueNDRangeKernel(command_queue, evalCondKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("enqueue evalue condition kernel clStatus = %d\n", clStatus);
	}

	clStatus = clEnqueueReadBuffer(command_queue, device_condition_val, CL_TRUE, 0, LOOP_SIZE * sizeof(int), host_condition_val, 0, NULL, NULL);

	if (DEBUG) {
		printf("read cond eval clStatus = %d\n", clStatus);
	}

	for (int i = 0; i < LOOP_SIZE; i++) {
		printf("%5d", host_condition_val[i]);
		if (i % 8 == 0) {
			puts("");
		}
	}

	clFlush(command_queue);
	clFinish(command_queue);
}


void BeforeCheckingExample2::initArrayValues() {
	if (DEBUG) {
		puts("init array values");
	}

	memset(host_condition_val, 0, sizeof(int) * LOOP_SIZE);

	for (int i = 0; i < LOOP_SIZE; i++) {
		if (i % 8 == 0) {
			host_c[i] = 1;
		} else {
			host_c[i] = 0;
		}
	}

}






