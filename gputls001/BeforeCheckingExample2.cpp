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
#include <algorithm>

const bool DEBUG = false;

struct IndexNode {
	int index;
	int condVal;

	bool operator <(const IndexNode& rhs) {
         return this->condVal < rhs.condVal || (this->condVal == rhs.condVal && this->index < rhs.index);
    } 
};


static size_t num0;

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
	delete[] host_buffer;
	delete[] host_index_node;

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

	check_loopkernel_0_writeon_a = clCreateKernel(program, "check_loopkernel_0_writeon_a", &clStatus);
	if (DEBUG) {
		printf("check_loopkernel_0_writeon_a status = %d\n", clStatus);
	}

	check_loopkernel_0_readon_a = clCreateKernel(program, "check_loopkernel_0_readon_a", &clStatus);
	if (DEBUG) {
		printf("check_loopkernel_0_readon_a status = %d\n", clStatus);
	}

	check_loopkernel_1_writeon_a = clCreateKernel(program, "check_loopkernel_1_writeon_a", &clStatus);
	if (DEBUG) {
		printf("check_loopkernel_1_writeon_a status = %d\n", clStatus);
	}

}



void BeforeCheckingExample2::assign_host_memory() {
	host_a = new float[ARRAY_SIZE];
	host_b = new float[ARRAY_SIZE];
	host_P = new int[ARRAY_SIZE];
	host_Q = new int[ARRAY_SIZE];
    host_buffer = new int[ARRAY_SIZE];


	host_c = new float[LOOP_SIZE];
    host_index_node = new IndexNode[LOOP_SIZE];

}


void BeforeCheckingExample2::assign_device_memory() {
	cl_int clStatus;

	device_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_a, &clStatus);
	device_b = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), host_b, &clStatus);
	
	device_P = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_P, &clStatus);
	device_Q = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_Q, &clStatus);

	device_c = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(int), host_c, &clStatus);

	host_raceFlag = 0;

	device_raceFlag = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &host_raceFlag, &clStatus);

	device_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_buffer, &clStatus);

	device_index_node = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, LOOP_SIZE * sizeof(IndexNode), host_index_node, &clStatus);

}



void BeforeCheckingExample2::destroy_device_memory() {
	clReleaseMemObject(device_a);
	clReleaseMemObject(device_b);
	clReleaseMemObject(device_c);

	clReleaseMemObject(device_P);
	clReleaseMemObject(device_Q);
	
	clReleaseMemObject(device_index_node);

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



void BeforeCheckingExample2::evaluateConditions() {
	if (DEBUG) {
		puts("evalute conditions");
	}

	cl_int clStatus ;
	int p = 0;
	clSetKernelArg(evalCondKernel, p++, sizeof(cl_mem), &device_c);
	clSetKernelArg(evalCondKernel, p++, sizeof(cl_mem), &device_index_node);
	clSetKernelArg(evalCondKernel, p++, sizeof(cl_int), &LOOP_SIZE);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;
	
	clStatus = clEnqueueNDRangeKernel(command_queue, evalCondKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("enqueue evalue condition kernel clStatus = %d\n", clStatus);
	}

	clStatus = clEnqueueReadBuffer(command_queue, device_index_node, CL_TRUE, 0, LOOP_SIZE * sizeof(IndexNode), host_index_node, 0, NULL, NULL);

	if (DEBUG) {
		printf("read cond eval clStatus = %d\n", clStatus);
		for (int i = 0; i < LOOP_SIZE; i++) {
			printf("%5d", host_index_node[i].condVal);
			if (i % 8 == 0) {
				puts("");
			}
		}

		puts("");
	}


	clFlush(command_queue);
	clFinish(command_queue);
}


void BeforeCheckingExample2::initArrayValues() {
	if (DEBUG) {
		puts("init array values");
	}

	num0 = 0;

	host_raceFlag = 0;

	memset(host_index_node, 0, sizeof(IndexNode) * LOOP_SIZE);

	memset(host_P, 0, sizeof(int) * ARRAY_SIZE);

	memset(host_buffer, 0, sizeof(int) * ARRAY_SIZE);

	memset(host_Q, 0, sizeof(int) * ARRAY_SIZE);

	memset(host_a, 0, sizeof(float) * ARRAY_SIZE);

	memset(host_b, 0, sizeof(float) * ARRAY_SIZE);

	for (int i = 0; i < LOOP_SIZE; i++) {
		if (i % 32 == 0) {
			host_c[i] = 1;
		} else {
			host_c[i] = 0;
			num0++;
		}
	}

	for (int i = 0; i < LOOP_SIZE; i++) {
		host_P[i] = i * 3;
		host_Q[i] = i * 3 + 1;
	}

	if (DEBUG) {
		puts("finish init array values");
	}
}

void BeforeCheckingExample2::sortBuildIndex() {
	if (DEBUG) {
		puts("sort and build Index array");
	}

	std::sort(host_index_node, host_index_node + LOOP_SIZE);

	if (DEBUG) {

		for (int i = 0; i < LOOP_SIZE; i++) {
			printf("(%d, %d) ", host_index_node[i].index, host_index_node[i].condVal);
			if (i % 10 == 0) {
				puts("");
			}
		}

		puts("\n");
	}

}

bool BeforeCheckingExample2::checkBranch0() {
	if (DEBUG) {
		puts("checkBranch0");
	}

	int clStatus;

	int p = 0;

	clSetKernelArg(check_loopkernel_0_writeon_a, p++, sizeof(cl_mem), &device_buffer);
	clSetKernelArg(check_loopkernel_0_writeon_a, p++, sizeof(cl_mem), &device_P);
	clSetKernelArg(check_loopkernel_0_writeon_a, p++, sizeof(cl_mem), &device_index_node);
	clSetKernelArg(check_loopkernel_0_writeon_a, p++, sizeof(cl_mem), &device_raceFlag);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;
	
	clStatus = clEnqueueNDRangeKernel(command_queue, check_loopkernel_0_writeon_a, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("check_loopkernel_0_writeon_a enqueuekernel status = %d\n", clStatus);
	}

	clStatus = clEnqueueReadBuffer(command_queue, device_raceFlag, CL_TRUE, 0, sizeof(int), &host_raceFlag, 0, NULL, NULL);

	if (DEBUG) {
		printf("check_loopkernel_0_writeon_a raceFlag = %d\n", host_raceFlag);

		clStatus = clEnqueueReadBuffer(command_queue, device_buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, host_buffer, 0, NULL, NULL);
		for (int i = 0 ; i < ARRAY_SIZE; i++) {
			printf("%d ", host_buffer[i]);

		}
		puts("");

	}

	if (host_raceFlag != 0) {
		return true;
	}


	p = 0;
	clSetKernelArg(check_loopkernel_0_readon_a, p++, sizeof(cl_mem), &device_buffer);
	clSetKernelArg(check_loopkernel_0_readon_a, p++, sizeof(cl_mem), &device_Q);
	clSetKernelArg(check_loopkernel_0_readon_a, p++, sizeof(cl_mem), &device_index_node);
	clSetKernelArg(check_loopkernel_0_readon_a, p++, sizeof(cl_mem), &device_raceFlag);

	clStatus = clEnqueueNDRangeKernel(command_queue, check_loopkernel_0_readon_a, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("check_loopkernel_0_readon_a enqueuekernel status = %d\n", clStatus);
	}

	clStatus = clEnqueueReadBuffer(command_queue, device_raceFlag, CL_TRUE, 0, sizeof(int), &host_raceFlag, 0, NULL, NULL);

	if (DEBUG) {
		printf("!! check_loopkernel_0_readon_a raceFlag = %d\n", host_raceFlag);

		clStatus = clEnqueueReadBuffer(command_queue, device_buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, host_buffer, 0, NULL, NULL);
		for (int i = 0 ; i < ARRAY_SIZE; i++) {
			printf("%d ", host_buffer[i]);

		}
		puts("");
	}

	if (host_raceFlag == 0) {
		return false;
	} else {
		return true;
	}
}

bool BeforeCheckingExample2::checkBranch1() {

	if (DEBUG) {
		puts("checkBranch1");
	}


	int clStatus;

	int p = 0;

	clSetKernelArg(check_loopkernel_1_writeon_a, p++, sizeof(cl_mem), &device_buffer);
	clSetKernelArg(check_loopkernel_1_writeon_a, p++, sizeof(cl_mem), &device_P);
	clSetKernelArg(check_loopkernel_1_writeon_a, p++, sizeof(cl_mem), &device_Q);
	clSetKernelArg(check_loopkernel_1_writeon_a, p++, sizeof(cl_mem), &device_index_node);
	clSetKernelArg(check_loopkernel_1_writeon_a, p++, sizeof(cl_mem), &device_raceFlag);

	size_t global_size = LOOP_SIZE;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, check_loopkernel_1_writeon_a, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("branch1 kernel status = %d\n", clStatus);
	}

	clStatus = clEnqueueReadBuffer(command_queue, device_raceFlag, CL_TRUE, 0, sizeof(int), &host_raceFlag, 0, NULL, NULL);

	if (DEBUG) {
		printf("check_loopkernel_1_writeon_a raceFlag = %d\n", host_raceFlag);

		clStatus = clEnqueueReadBuffer(command_queue, device_buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, host_buffer, 0, NULL, NULL);

		for (int i = 0 ; i < ARRAY_SIZE; i++) {
			printf("%d ", host_buffer[i]);

		}

		puts("");
	}

	if (host_raceFlag == 0) {
		return false;
	} else {
		return true;
	}

}


bool BeforeCheckingExample2::parallelCheck() {
	if (!checkBranch0()) {
		clear_writebuffer();
		if (!checkBranch1()) {
			if (DEBUG) {
				puts("no data race");
			}
			return false;
		}
		return true;
	} else {
		return true;
	}

}


void BeforeCheckingExample2::clear_writebuffer() {
	int clStatus;
	clReleaseMemObject(device_buffer);
	memset(host_buffer, 0, sizeof(int) * ARRAY_SIZE);
	device_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(int), host_buffer, &clStatus);
}



void BeforeCheckingExample2::parallelExecute() {
	//loopKernel0, loopKernel1

	/*struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);
	*/

	int clStatus;
	int p = 0;
	clSetKernelArg(loopKernel0, p++, sizeof(cl_mem), &device_a );
	clSetKernelArg(loopKernel0, p++, sizeof(cl_mem), &device_b );
	clSetKernelArg(loopKernel0, p++, sizeof(cl_mem), &device_Q );
	clSetKernelArg(loopKernel0, p++, sizeof(cl_mem), &device_P );
	clSetKernelArg(loopKernel0, p++, sizeof(cl_mem), &device_index_node );
	clSetKernelArg(loopKernel0, p++, sizeof(cl_int), &LOOP_SIZE );
	clSetKernelArg(loopKernel0, p++, sizeof(cl_int), &CALC_SIZE );

	p = 0;
	clSetKernelArg(loopKernel1, p++, sizeof(cl_mem), &device_a );
	clSetKernelArg(loopKernel1, p++, sizeof(cl_mem), &device_b );
	clSetKernelArg(loopKernel1, p++, sizeof(cl_mem), &device_Q );
	clSetKernelArg(loopKernel1, p++, sizeof(cl_mem), &device_P );
	clSetKernelArg(loopKernel1, p++, sizeof(cl_mem), &device_index_node );
	clSetKernelArg(loopKernel1, p++, sizeof(cl_int), &LOOP_SIZE );
	clSetKernelArg(loopKernel1, p++, sizeof(cl_int), &CALC_SIZE );

	size_t global_size0 = num0;
	size_t local_size = 64;


	size_t global_size1 = LOOP_SIZE - num0;


	clStatus = clEnqueueNDRangeKernel(command_queue, loopKernel0, 1, NULL, &global_size0, &local_size, 0, NULL, NULL);

	clStatus = clEnqueueNDRangeKernel(command_queue, loopKernel1, 1, &num0, &global_size1, &local_size, 0, NULL, NULL);

	if (DEBUG) {
		printf("%d\n", clStatus);
	}

	clFlush(command_queue);
	clFinish(command_queue);


	/*gettimeofday(&tv2, NULL);
	double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	printf("time after expand branch = %.2f\n", used_time);
	*/
}


void BeforeCheckingExample2::parallelExecuteOrigin() {
	/*struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);
	*/

	int p = 0;
	int clStatus;

	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_a );
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_b );
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_c );
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_Q );
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_mem), &device_P );
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_int), &LOOP_SIZE );
	clSetKernelArg(loopKernelOrigin, p++, sizeof(cl_int), &CALC_SIZE );

	size_t global_size0 = LOOP_SIZE;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, loopKernelOrigin, 1, NULL, &global_size0, &local_size, 0, NULL, NULL);

	clFlush(command_queue);
	clFinish(command_queue);

	/*
	gettimeofday(&tv2, NULL);
	double used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	printf("time origin = %.2f\n", used_time);
	*/

}


