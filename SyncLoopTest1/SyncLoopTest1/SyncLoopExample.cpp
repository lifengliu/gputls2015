
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

#include <fstream>
#include <streambuf>
#include <map>
#include "ParallelBitonicASort.h"
#include "ParallelBitonicLocalSort.h"
#include "ParallelBitonicCSort.h"


using std::fill;
using std::map;
using std::make_pair;
using std::string;


const static bool DEBUG = false;


SyncLoopExample::SyncLoopExample(const OpenCLRuntimeEnv& env, string kernelSourceCode, int loopsize, int calcSize1, int calsSize2) :
	loopsize(loopsize),
	calcSize1(calcSize1),
	calcSize2(calcSize2)
{

	this->kernelSourceCode = kernelSourceCode;
	this->env = env;

	init_opencl_resources();
}

SyncLoopExample::~SyncLoopExample() {
	release_opencl_resources();
}

void SyncLoopExample::init0()
{
	
	assign_host_memory();
	init_host_memory();
	assign_device_memory();
}

void SyncLoopExample::destroy0()
{
	destroy_device_memory();
	destroy_host_memory();
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
		delete[] log;
		return;
	}

}

void SyncLoopExample::init_host_memory()
{
	memset(host_a, 0, sizeof(int) * loopsize * 2);

	/*for (int i = 0; i < loopsize / 2; i++) {
		host_c[i] = 100;
	}

	for (int i = loopsize / 2; i < loopsize; i++) {
		host_c[i] = -100;
	}*/

	for (int i = 0; i < loopsize; i++) {
		if (i % 2 == 0) {
			host_c[i] = 100;
		}
		else {
			host_c[i] = -100;
		}
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

	memset(host_indexnode, 0, sizeof(data_t) * loopsize);

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
	clSetKernelArg(loopKernelRemapped, 7, sizeof(cl_mem), &dev_indexnodeout);

	clEnqueueNDRangeKernel(env.get_command_queue(), loopKernelRemapped, 1, NULL, global_work_size, block_size, 0, NULL, NULL);

	clFinish(env.get_command_queue());

	

	auto end = std::chrono::high_resolution_clock::now(); //end measurement here
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	putToTimer("remappedLoopGPU", elapsedtime);
	//timer["remappedLoopGPU"] = elapsedtime;

	if (DEBUG) {
		std::cout << "remapped loop gpu = " << elapsedtime << "ms" << std::endl;
	}

}

static string loadFile(const string& fileLoc) {
	std::ifstream t1(fileLoc);
	string str((std::istreambuf_iterator<char>(t1)), std::istreambuf_iterator<char>());
	return str;
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

	

}

void SyncLoopExample::partialSort(int wg) {

	std::string s1 = loadFile("SortKernels.cl");
	ParallelBitonicLocalSort psort(env, wg, s1);
	
	auto start = std::chrono::high_resolution_clock::now();
	psort.sort(loopsize, dev_indexnode, dev_indexnodeout);

	auto end = std::chrono::high_resolution_clock::now();
	auto cpu_elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	data_t *host_tmp = new data_t[loopsize];

	clEnqueueReadBuffer(env.get_command_queue(), dev_indexnodeout, CL_TRUE, 0, loopsize * sizeof(data_t), host_tmp, 0, NULL, NULL);

	int badsum = 0;
	int C = 64;

	for (int i = 0; i < loopsize; i += C) {
		bool flag = false;

		for (int j = i; j < std::min(i + C, loopsize); j++) {
			if (j + 1 < std::min(i + C, loopsize) && getKey(host_tmp[j]) != getKey(host_tmp[j + 1])) {
				flag = true;
			}
		}

		if (flag) {
			badsum++;
		}
	}

	printf("badsum = %d\n", badsum);


	for (int i = 0; i < 100; i++) {
		printf("%d %d\n", getKey(host_tmp[i]), getValue(host_tmp[i]));
	}

	putToTimer("partialSort" + std::to_string(wg), cpu_elapsedtime);

	//timer["partialSort"] = cpu_elapsedtime;

	delete[] host_tmp;

	if (DEBUG) {
		std::cout << "partial sort" << cpu_elapsedtime << "ns" << std::endl;
	}
}


void SyncLoopExample::fullySort() {

	std::string s1 = loadFile("SortKernels.cl");
	
	ParallelBitonicCSort psort(env, 256, s1);

	auto start = std::chrono::high_resolution_clock::now();
	psort.sort(loopsize, dev_indexnode, dev_indexnodeout);

	auto end = std::chrono::high_resolution_clock::now();
	auto cpu_elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	data_t *host_tmp = new data_t[loopsize];

	clEnqueueReadBuffer(env.get_command_queue(), dev_indexnodeout, CL_TRUE, 0, loopsize * sizeof(data_t), host_tmp, 0, NULL, NULL);


	int badsum = 0;
	int C = 64;

	for (int i = 0; i < loopsize; i += C) {
		bool flag = false;
		
		for (int j = i; j < std::min(i + C, loopsize); j++) {
			if (j + 1 < std::min(i + C, loopsize) && getKey(host_tmp[j]) != getKey(host_tmp[j + 1])) {
				flag = true;
			}
		}
		
		if (flag) {
			badsum++;
		}
	}

	printf("badsum = %d\n", badsum);

	for (int i = 0; i < 100; i++) {
		printf("%d %d\n", getKey(host_tmp[i]), getValue(host_tmp[i]));
	}


	timer["fullySort"] = cpu_elapsedtime;

	delete[] host_tmp;

	if (DEBUG) {
		std::cout << "fully sort" << cpu_elapsedtime << "ns" << std::endl;
	}

}


void SyncLoopExample::dependencyChecking()
{
	auto start = std::chrono::high_resolution_clock::now();
	dc_write_on_a();
	dc_read_on_a();
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	timer["dc"] = elapsedtime;
}


const map<string, long long>& SyncLoopExample::getTimer() const
{
	return timer;
}


void SyncLoopExample::assign_host_memory() {
	host_c = new int[loopsize];
	host_a = new int[loopsize * 2];
	host_P = new int[loopsize];
	host_Q = new int[loopsize];
	host_indexnode = new data_t[loopsize];
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
	
	//dev_c = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_c, &clStatus);
	//dev_a = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * 2 * sizeof(int), host_a, &clStatus);
	//dev_P = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_P, &clStatus);
	//dev_Q = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(int), host_Q, &clStatus);
	
	dev_c = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * sizeof(int), NULL, &clStatus);
	dev_a = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * 2 * sizeof(int), NULL, &clStatus);
	dev_P = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * sizeof(int), NULL, &clStatus);
	dev_Q = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * sizeof(int), NULL, &clStatus);

	clEnqueueWriteBuffer(env.get_command_queue(), dev_c, CL_TRUE, 0, loopsize *sizeof(int), host_c, 0, NULL, NULL);
	clEnqueueWriteBuffer(env.get_command_queue(), dev_a, CL_TRUE, 0, loopsize * 2 * sizeof(int), host_a, 0, NULL, NULL);
	clEnqueueWriteBuffer(env.get_command_queue(), dev_P, CL_TRUE, 0, loopsize * sizeof(int), host_P, 0, NULL, NULL);
	clEnqueueWriteBuffer(env.get_command_queue(), dev_Q, CL_TRUE, 0, loopsize * sizeof(int), host_Q, 0, NULL, NULL);


	dev_raceflag = clCreateBuffer(env.get_context(), CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &raceFlag, &clStatus);

	//dev_indexnode = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, loopsize * sizeof(data_t), host_indexnode, &clStatus);
	
	dev_indexnode = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * sizeof(data_t), NULL, &clStatus);

	dev_indexnodeout = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * sizeof(data_t), NULL, &clStatus);

	dev_buffer = clCreateBuffer(env.get_context(), CL_MEM_READ_WRITE, loopsize * 2 * sizeof(int), NULL, &clStatus);
	
}

void SyncLoopExample::destroy_device_memory() {
	clReleaseMemObject(dev_c);
	clReleaseMemObject(dev_a);
	clReleaseMemObject(dev_P);
	clReleaseMemObject(dev_Q);
	clReleaseMemObject(dev_indexnode);
	clReleaseMemObject(dev_indexnodeout);
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

	//timer["dcwriteonaKernel"] = elapsedtime;

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

	//timer["dcreadonaKernel"] = elapsedtime;

	if (DEBUG) {
		std::cout << "dcreadonaKernel" << elapsedtime << "ms" << std::endl;
	}

}

void SyncLoopExample::putToTimer(string s, long long v)
{
	
	if (timer.count(s) == 0) {
		timer[s] = v;
		return;
	}

	int t = 0;
	
	string s1 = s + std::to_string(t);
	while (timer.count(s1) != 0) {
		s1 = s + std::to_string(++t);
	}

	printf("%s\n", (s1).c_str());
	
	timer[s1] = v;
}

