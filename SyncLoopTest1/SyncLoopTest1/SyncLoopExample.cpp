
#pragma warning(disable : 4996)
#include "SyncLoopExample.h"

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


const static bool DEBUG = true;
static map<string, long long> timer;

/*struct IndexNode {
	int index;
	int condVal;

	bool operator <(const IndexNode& rhs) {
		return this->condVal < rhs.condVal || (this->condVal == rhs.condVal && this->index < rhs.index);
	}
};*/


struct IndexNode1 {
	int groupid;
	int localid;
	int condVal;
	bool operator <(const IndexNode1& rhs) {
		return this->condVal < rhs.condVal;
	}
};



SyncLoopExample::SyncLoopExample(const OpenCLRuntimeEnv& env, string kernelSourceCode, int loopsize, int calcSize1, int calsSize2) :
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

SyncLoopExample::~SyncLoopExample() {
	destroy_device_memory();
	destroy_host_memory();
	release_opencl_resources();

	printf("%s\n", "I destroy myself");

}


void SyncLoopExample::init_opencl_resources() {
	cl_int clStatus;
	
	const char *program_source = kernelSourceCode.c_str();
	
	program = clCreateProgramWithSource(env.get_context(), 1, (const char **) &program_source, NULL, &clStatus);

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

void SyncLoopExample::init_host_memory()
{
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


	memset(host_indexnode, 0, sizeof(IndexNode1) * loopsize);

} 


static int comp1(int CALC_SIZE) {
	int res = 0;
	for (int p = 1; p <= CALC_SIZE; p++) {
		res = (p + res) % 1047;
	}

	return res;
}

static int comp2(int CALC_SIZE) {
	int res = 0;
	for (int p = CALC_SIZE; p >= 1; p--) {
		res = (p + res) % 1031;
	}

	return res;
}



void SyncLoopExample::sequentialCPU()
{

	init_host_memory();

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here

	for (int i = 0; i < loopsize; i++) {
		if (host_c[i] > 0) {
			host_a[host_Q[i]] = comp1(calcSize1) + host_a[host_P[i]];
		} else {
			host_a[host_Q[i]] = comp2(calcSize2);
		}
	}

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	timer["seqCPU"] = elapsedtime;

	if (DEBUG) {
		std::cout << "sequential" <<  elapsedtime << "ms" << std::endl;
	}

}

void SyncLoopExample::unremappedGPU()
{
	init_host_memory();

	int clStatus;
	loopKernelOrigin = clCreateKernel(program, "loop_task_kernel", &clStatus);
	if (DEBUG) {
		printf("create loop task kernel = %d\n", clStatus);
	}

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here

	#define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
	size_t global_work_size[1] = { (loopsize <= 1048544 ? floord(loopsize + 31, 32) : 32768) * 32 };
	size_t block_size[1] = { 32 };

	clSetKernelArg(loopKernelOrigin, 0, sizeof(cl_mem), (void *)&dev_P);
	clSetKernelArg(loopKernelOrigin, 1, sizeof(cl_mem), (void *)&dev_Q);
	clSetKernelArg(loopKernelOrigin, 2, sizeof(cl_mem), (void *)&dev_a);
	clSetKernelArg(loopKernelOrigin, 3, sizeof(cl_mem), (void *)&dev_c);
	clSetKernelArg(loopKernelOrigin, 4, sizeof(int), &loopsize);
	clSetKernelArg(loopKernelOrigin, 5, sizeof(int), &calcSize1);
	clSetKernelArg(loopKernelOrigin, 6, sizeof(int), &calcSize2);
	
	clEnqueueNDRangeKernel(env.get_command_queue(), loopKernelOrigin, 1, NULL, global_work_size, block_size, 0, NULL, NULL);


	clFinish(env.get_command_queue());

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	
	timer["unremappedGPU"] = elapsedtime;

	if (DEBUG) {
		std::cout << "unremapped kernel gpu = " << elapsedtime << "ms" << std::endl;
	}

}




void SyncLoopExample::remappedGPU()
{
	int clStatus;
	loopKernelRemapped = clCreateKernel(program, "loop_task_kernel_remapped", &clStatus);

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here

#define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
	size_t global_work_size[1] = { (loopsize <= 1048544 ? floord(loopsize + 31, 32) : 32768) * 32 };
	size_t block_size[1] = { 32 };

	clSetKernelArg(loopKernelRemapped, 0, sizeof(cl_mem), (void *)&dev_P);
	clSetKernelArg(loopKernelRemapped, 1, sizeof(cl_mem), (void *)&dev_Q);
	clSetKernelArg(loopKernelRemapped, 2, sizeof(cl_mem), (void *)&dev_a);
	clSetKernelArg(loopKernelRemapped, 3, sizeof(cl_mem), (void *)&dev_c);
	clSetKernelArg(loopKernelRemapped, 4, sizeof(int), &loopsize);
	clSetKernelArg(loopKernelRemapped, 5, sizeof(int), &calcSize1);
	clSetKernelArg(loopKernelRemapped, 6, sizeof(int), &calcSize2);
	clSetKernelArg(loopKernelRemapped, 7, sizeof(cl_mem), &dev_indexnode);

	clEnqueueNDRangeKernel(env.get_command_queue(), loopKernelRemapped, 1, NULL, global_work_size, block_size, 0, NULL, NULL);

	clFinish(env.get_command_queue());

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	timer["remappedLoopGPU"] = elapsedtime;

	if (DEBUG) {
		std::cout << "remapped loop gpu = " << elapsedtime << "ms" << std::endl;
	}

}

void SyncLoopExample::evaluateBranch()
{
	int clStatus;
	init_host_memory();
	branchEvaluateKernel = clCreateKernel(program, "branch_evaluate_task_kernel", &clStatus);
	
	if (DEBUG) {
		printf("create loop task kernel = %d\n", clStatus);
	}

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here

#define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
	size_t global_work_size[1] = { (loopsize <= 1048544 ? floord(loopsize + 31, 32) : 32768) * 32 };
	size_t block_size[1] = { 32 };

	clSetKernelArg(branchEvaluateKernel, 0, sizeof(cl_mem), (void *)&dev_P);
	clSetKernelArg(branchEvaluateKernel, 1, sizeof(cl_mem), (void *)&dev_Q);
	clSetKernelArg(branchEvaluateKernel, 2, sizeof(cl_mem), (void *)&dev_a);
	clSetKernelArg(branchEvaluateKernel, 3, sizeof(cl_mem), (void *)&dev_c);
	clSetKernelArg(branchEvaluateKernel, 4, sizeof(int), &loopsize);
	clSetKernelArg(branchEvaluateKernel, 5, sizeof(int), &calcSize1);
	clSetKernelArg(branchEvaluateKernel, 6, sizeof(int), &calcSize2);
	clSetKernelArg(branchEvaluateKernel, 7, sizeof(cl_mem), &dev_indexnode);

	clEnqueueNDRangeKernel(env.get_command_queue(), branchEvaluateKernel, 1, NULL, global_work_size, block_size, 0, NULL, NULL);

	clFinish(env.get_command_queue());

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	timer["evaluateBranchGPU"] = elapsedtime;

	if (DEBUG) {
		std::cout << "evaluate" << elapsedtime << "ms" << std::endl;
	}

	// need GPU sorting

	start = std::chrono::high_resolution_clock::now();
	clEnqueueReadBuffer(env.get_command_queue(), dev_indexnode, CL_TRUE, 0, loopsize * sizeof(IndexNode1), host_indexnode, 0, NULL, NULL);

	std::sort(host_indexnode, host_indexnode + loopsize);

	clEnqueueWriteBuffer(env.get_command_queue(), dev_indexnode, CL_TRUE, 0, loopsize * sizeof(IndexNode1), host_indexnode, 0, NULL, NULL);
	end = std::chrono::high_resolution_clock::now();
	elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	timer["sort"] = elapsedtime;

	if (DEBUG) {
		std::cout << "sort" << elapsedtime << "ms" << std::endl;
	}

}


void SyncLoopExample::dependencyChecking()
{
	dc_write_on_a();
	dc_read_on_a();
}


void SyncLoopExample::assign_host_memory() {
	host_c = new int[loopsize];
	host_a = new int[loopsize * 2];
	host_P = new int[loopsize];
	host_Q = new int[loopsize];
	host_indexnode = new IndexNode1[loopsize];
}

void SyncLoopExample::destroy_host_memory()
{
	delete[] host_c;
	delete[] host_a;
	delete[] host_P;
	delete[] host_Q;
	delete[] host_indexnode;
}


void SyncLoopExample::assign_device_memory() {
	int clStatus;
	
	dev_c = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_c, &clStatus);
	dev_a = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * 2 * sizeof(int), host_a, &clStatus);
	dev_P = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_P, &clStatus);
	dev_Q = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_Q, &clStatus);
	
	dev_raceflag = clCreateBuffer(env.get_context(), CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &raceFlag, &clStatus);

	dev_indexnode = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(IndexNode1), host_indexnode, &clStatus);
	dev_buffer = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * 2 * sizeof(int), NULL, &clStatus);
	
}

void SyncLoopExample::destroy_device_memory() {
	clReleaseMemObject(dev_c);
	clReleaseMemObject(dev_a);
	clReleaseMemObject(dev_P);
	clReleaseMemObject(dev_Q);
	clReleaseMemObject(dev_indexnode);
	clReleaseMemObject(dev_buffer);
	clReleaseMemObject(dev_raceflag);
}

void SyncLoopExample::release_opencl_resources() {
	clReleaseKernel(loopKernelOrigin);
	clReleaseKernel(loopKernelRemapped);
	clReleaseKernel(branchEvaluateKernel);
	clReleaseKernel(dcwriteonaKernel);
	clReleaseKernel(dcreadonaKernel);
	clReleaseProgram(program);
}

void SyncLoopExample::dc_write_on_a()
{
	int clStatus;
	dcwriteonaKernel = clCreateKernel(program, "dc_write_on_a", &clStatus);
	
	int zero = 0;
	clEnqueueFillBuffer(env.get_command_queue(), dev_buffer, &zero, sizeof(int), 0, sizeof(int) * loopsize * 2, 0, NULL, NULL);
	clEnqueueFillBuffer(env.get_command_queue(), dev_raceflag, &zero, sizeof(int), 0, sizeof(int), 0, NULL, NULL);

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here

#define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
	size_t global_work_size[1] = { (loopsize <= 1048544 ? floord(loopsize + 31, 32) : 32768) * 32 };
	size_t block_size[1] = { 32 };

	clSetKernelArg(dcwriteonaKernel, 0, sizeof(cl_mem), (void *)&dev_P);
	clSetKernelArg(dcwriteonaKernel, 1, sizeof(cl_mem), (void *)&dev_Q);
	clSetKernelArg(dcwriteonaKernel, 2, sizeof(cl_mem), (void *)&dev_a);
	clSetKernelArg(dcwriteonaKernel, 3, sizeof(cl_mem), (void *)&dev_c);
	clSetKernelArg(dcwriteonaKernel, 4, sizeof(int), &loopsize);
	clSetKernelArg(dcwriteonaKernel, 5, sizeof(int), &calcSize1);
	clSetKernelArg(dcwriteonaKernel, 6, sizeof(int), &calcSize2);
	clSetKernelArg(dcwriteonaKernel, 7, sizeof(cl_mem), &dev_raceflag);
	clSetKernelArg(dcwriteonaKernel, 8, sizeof(cl_mem), &dev_buffer);


	clEnqueueNDRangeKernel(env.get_command_queue(), dcwriteonaKernel, 1, NULL, global_work_size, block_size, 0, NULL, NULL);

	clFinish(env.get_command_queue());

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	timer["dcwriteonaKernel"] = elapsedtime;

	if (DEBUG) {
		std::cout << "dcwriteonaKernel" << elapsedtime << "ms" << std::endl;
	}

	//clReleaseKernel(dcwriteonaKernel);
	
}

void SyncLoopExample::dc_read_on_a()
{
	int clStatus;
	dcreadonaKernel = clCreateKernel(program, "dc_read_on_a", &clStatus);

	auto start = std::chrono::high_resolution_clock::now(); //measure time starting here

#define floord(n,d) (((n)<0) ? -((-(n)+(d)-1)/(d)) : (n)/(d))
	size_t global_work_size[1] = { (loopsize <= 1048544 ? floord(loopsize + 31, 32) : 32768) * 32 };
	size_t block_size[1] = { 32 };

	clSetKernelArg(dcreadonaKernel, 0, sizeof(cl_mem), (void *)&dev_P);
	clSetKernelArg(dcreadonaKernel, 1, sizeof(cl_mem), (void *)&dev_Q);
	clSetKernelArg(dcreadonaKernel, 2, sizeof(cl_mem), (void *)&dev_a);
	clSetKernelArg(dcreadonaKernel, 3, sizeof(cl_mem), (void *)&dev_c);
	clSetKernelArg(dcreadonaKernel, 4, sizeof(int), &loopsize);
	clSetKernelArg(dcreadonaKernel, 5, sizeof(int), &calcSize1);
	clSetKernelArg(dcreadonaKernel, 6, sizeof(int), &calcSize2);
	clSetKernelArg(dcreadonaKernel, 7, sizeof(cl_mem), &dev_raceflag);
	clSetKernelArg(dcreadonaKernel, 8, sizeof(cl_mem), &dev_buffer);


	clEnqueueNDRangeKernel(env.get_command_queue(), dcreadonaKernel, 1, NULL, global_work_size, block_size, 0, NULL, NULL);

	clFinish(env.get_command_queue());

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	timer["dcreadonaKernel"] = elapsedtime;

	if (DEBUG) {
		std::cout << "dcreadonaKernel" << elapsedtime << "ms" << std::endl;
	}

}

