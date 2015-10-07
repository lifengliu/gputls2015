#include "PGExample.h"
#include <CL/opencl.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include "OpenCLRuntimeEnv.h"
#include <cstring>
#include <chrono>
#include <ctime>
#include <string>
#include <iostream>

#include <map>

using std::fill;
using std::map;
using std::make_pair;
using std::string;

typedef struct TraceSet {
	int size;
	int indices[5];  // record index, not address
} TraceSet;



const static bool DEBUG = true;




PGExample::PGExample(const OpenCLRuntimeEnv & env, string kernelSourceCode, int loopsize, int calcSize1, int calcSize2):
	loopsize(loopsize),
	calcSize1(calcSize1),
	calcSize2(calcSize2)
{
	this->kernelSourceCode = kernelSourceCode;
	this->env = env;

	init_opencl_resources();

	assign_host_memory();
	init_host_memory();
	assign_device_memory();

}


PGExample::~PGExample()
{
}

void PGExample::specExecute()
{
	
	int clStatus;
	loopSpecKernel = clCreateKernel(program, "loop_task_spec_kernel", &clStatus);

	
	cl_uint p = 0;
	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_mem), &dev_P);
	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_mem), &dev_Q);
	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_mem), &dev_a);
	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_mem), &dev_c);


	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_int), &loopsize);
	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_int), &calcSize1);
	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_int), &calcSize2);

	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_mem), &device_read_trace_a);
	clSetKernelArg(loopSpecKernel, p++, sizeof(cl_mem), &device_write_trace_a);

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here
	int zero = 0;
	clEnqueueFillBuffer(env.get_command_queue(), device_read_trace_a, &zero, sizeof(int), 0, sizeof(TraceSet) * loopsize, 0, NULL, NULL);
	clEnqueueFillBuffer(env.get_command_queue(), device_write_trace_a, &zero, sizeof(int), 0, sizeof(TraceSet) * loopsize, 0, NULL, NULL);
	clEnqueueFillBuffer(env.get_command_queue(), device_read_to, &zero, sizeof(int), 0, sizeof(int) * loopsize * 2, 0, NULL, NULL);
	clEnqueueFillBuffer(env.get_command_queue(), device_write_to, &zero, sizeof(int), 0, sizeof(int) * loopsize * 2, 0, NULL, NULL);
	clEnqueueFillBuffer(env.get_command_queue(), device_write_count, &zero, sizeof(int), 0, sizeof(int) * loopsize * 2, 0, NULL, NULL);


#define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
	size_t global_work_size[1] = { (loopsize <= 1048544 ? floord(loopsize + 31, 32) : 32768) * 32 };
	size_t block_size[1] = { 32 };

	clEnqueueNDRangeKernel(env.get_command_queue(), loopSpecKernel, 1, NULL, global_work_size, block_size, 0, NULL, NULL);
	clFinish(env.get_command_queue());

	if (DEBUG) {
		printf("enqueue nd range kernel clStatus = %d\n", clStatus);
	}

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	timer["specloopexec"] = elapsedtime;

	if (DEBUG) {
		std::cout << "specloopexec" << elapsedtime << "ms" << std::endl;
	}

	
}

void PGExample::dependencyChecking()
{
	dc1();
	dc2();
	dc3();
}

const map<string, long long> PGExample::getTimer() const
{
	return timer;
}



void PGExample::assign_host_memory()
{
	host_c = new int[loopsize];
	host_a = new int[2 * loopsize];
	host_P = new int[loopsize];
	host_Q = new int[loopsize];

	host_read_to = new int[2 * loopsize];
	host_write_to = new int[2 * loopsize];
	host_write_count = new int[2 * loopsize];

}

void PGExample::destroy_host_memory()
{
	delete[] host_c;
	delete[] host_a;
	delete[] host_P;
	delete[] host_Q;

	delete[] host_read_to;
	delete[] host_write_to;
	delete[] host_write_count;
}


void PGExample::init_host_memory()
{
	memset(host_read_to, 0, sizeof(int) * loopsize * 2);
	memset(host_write_to, 0, sizeof(int) * loopsize * 2);
	memset(host_write_count, 0, sizeof(int) * loopsize * 2);

	memset(host_a, 0, sizeof(int) * loopsize * 2);

	for (int i = 0; i < loopsize / 2; i++) {
		host_c[i] = 100;
	}

	for (int i = loopsize / 2; i < loopsize; i++) {
		host_c[i] = -100;
	}

	std::random_shuffle(host_c, host_c + loopsize);

	for (int i = 0; i < loopsize; i++) {
		host_Q[i] = i;
	}

	std::random_shuffle(host_Q, host_Q + loopsize);

	for (int i = loopsize; i < loopsize * 2; i++) {
		host_P[i - loopsize] = i;
	}

	std::random_shuffle(host_P, host_P + loopsize);
}


void PGExample::assign_device_memory()
{
	int clStatus;

	dev_c = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_c, &clStatus);
	dev_a = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * 2 * sizeof(int), host_a, &clStatus);
	dev_P = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_P, &clStatus);
	dev_Q = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_Q, &clStatus);
	dev_raceflag = clCreateBuffer(env.get_context(), CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &raceFlag, &clStatus);
	
	device_read_trace_a = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * sizeof(TraceSet), NULL, &clStatus);
	device_write_trace_a = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * sizeof(TraceSet), NULL, &clStatus);
	
	device_read_to = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * 2 * sizeof(int), NULL, &clStatus);
	device_write_to = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * 2 * sizeof(int), NULL, &clStatus);
	device_write_count = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * 2 * sizeof(int), NULL, &clStatus);;


}

void PGExample::destroy_device_memory()
{
	clReleaseMemObject(dev_c);
	clReleaseMemObject(dev_a);
	clReleaseMemObject(dev_P);
	clReleaseMemObject(dev_Q);

	clReleaseMemObject(dev_raceflag);
	clReleaseMemObject(device_read_trace_a);
	clReleaseMemObject(device_write_trace_a);
	clReleaseMemObject(device_read_to);
	clReleaseMemObject(device_write_to);
	clReleaseMemObject(device_write_count);
}


void PGExample::init_opencl_resources()
{
	cl_int clStatus;

	const char *program_source = kernelSourceCode.c_str();

	program = clCreateProgramWithSource(env.get_context(), 1, (const char **)&program_source, NULL, &clStatus);

	if (DEBUG) {
		printf("%d\n", clStatus);
	}

	cl_device_id device = env.get_device_id();

	clStatus = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	if (clStatus != CL_SUCCESS) {
		printf("clStatus = %d\n", clStatus);
		puts("build Program Error");
		size_t len;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &len);
		char *log = new char[len];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len, log, NULL);
		printf("%s\n", log);
	}


}

void PGExample::release_opencl_resources()
{
	clReleaseProgram(program);
}

void PGExample::dc1()
{
	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here
	int clStatus;
	dc1Kernel = clCreateKernel(program, "dc_phase1", &clStatus);

	int p = 0;

	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_read_trace_a);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_write_trace_a);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_read_to);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_write_to);
	clSetKernelArg(dc1Kernel, p++, sizeof(cl_mem), &device_write_count);

	size_t global_size = loopsize;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(env.get_command_queue(), dc1Kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	clFinish(env.get_command_queue());

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	timer["dc1"] = elapsedtime;

	if (DEBUG) {
		std::cout << "dc1 = " << elapsedtime << "ms" << std::endl;
	}


}

void PGExample::dc2()
{
	int clStatus;
	dc2reduceKernel = clCreateKernel(program, "reduce", &clStatus);

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here
	int writeToSum = dc_reduce(device_write_to, loopsize * 2);
	int writeCountSum = dc_reduce(device_write_count, loopsize);
	std::cout << writeToSum << "   " << writeCountSum << std::endl;
	
	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	if (DEBUG) {
		std::cout << "dc2 = " << elapsedtime << "ms" << std::endl;
	}

	timer["dc2"] = elapsedtime;
}


void PGExample::dc3()
{
	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here
	cl_int clStatus;
	dc3Kernel = clCreateKernel(program, "dc_phase3", &clStatus);

	int spec = 0;
	cl_mem device_misspeculation = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &spec, &clStatus);
	clSetKernelArg(dc3Kernel, 0, sizeof(cl_mem), &device_read_to);
	clSetKernelArg(dc3Kernel, 1, sizeof(cl_mem), &device_write_to);
	clSetKernelArg(dc3Kernel, 2, sizeof(cl_mem), &device_misspeculation);
	int arraysize = loopsize * 2;
	clSetKernelArg(dc3Kernel, 3, sizeof(cl_mem), &arraysize);

	size_t global_size = loopsize;
	size_t local_size = 64;

	clEnqueueNDRangeKernel(env.get_command_queue(), dc3Kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	clFinish(env.get_command_queue());
	clEnqueueReadBuffer(env.get_command_queue(), device_misspeculation, CL_TRUE, 0, sizeof(int), &spec, 0, NULL, NULL);
	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	
	if (DEBUG) {
		std::cout << "dc3 = " << elapsedtime << "ms" << std::endl;
	}

	timer["dc3"] = elapsedtime;
}

int PGExample::dc_reduce(cl_mem &reduced_array, int length) {
	cl_int clStatus;

	size_t localWorkSize = 128;
	size_t numWorkGroups = 64;
	size_t globalWorkSize = numWorkGroups * localWorkSize;

	cl_mem device_reducedWriteTo = clCreateBuffer(env.get_context(), CL_MEM_WRITE_ONLY, numWorkGroups * sizeof(int), NULL, &clStatus);

	clSetKernelArg(dc2reduceKernel, 0, sizeof(cl_mem), &reduced_array);
	clSetKernelArg(dc2reduceKernel, 1, sizeof(cl_int) * localWorkSize, NULL);
	clSetKernelArg(dc2reduceKernel, 2, sizeof(cl_int), &length);
	clSetKernelArg(dc2reduceKernel, 3, sizeof(cl_mem), &device_reducedWriteTo);

	clStatus = clEnqueueNDRangeKernel(env.get_command_queue(), dc2reduceKernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);

	int *reduced_writeToResult = new int[numWorkGroups];
	clEnqueueReadBuffer(env.get_command_queue(), device_reducedWriteTo, CL_TRUE, 0, sizeof(int) * numWorkGroups, reduced_writeToResult, 0, NULL, NULL);

	int writeToSum = 0;
	for (size_t i = 0; i < numWorkGroups; i++) {
		writeToSum += reduced_writeToResult[i];
		//printf("%d ", reduced_writeToResult[i]);
	}

	//printf("%d\n", writeToSum);

	clReleaseMemObject(device_reducedWriteTo);
	delete[] reduced_writeToResult;

	return writeToSum;
}