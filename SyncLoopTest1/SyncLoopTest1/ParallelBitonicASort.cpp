#include "ParallelBitonicASort.h"

#pragma warning(disable : 4996)

#include <chrono>
#include <iostream>

ParallelBitonicASort::ParallelBitonicASort(const OpenCLRuntimeEnv & env, std::string src):
	env(env), src(src)
{
	int clStatus;
	this->src = src;

	const char *program_source = src.c_str();

	program = clCreateProgramWithSource(env.get_context(), 1, (const char **)&program_source, NULL, &clStatus);

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

	kernel = clCreateKernel(program, "ParallelBitonic_A", &clStatus);
}

ParallelBitonicASort::~ParallelBitonicASort()
{
	clReleaseKernel(kernel);
	clReleaseProgram(program);
}



bool ParallelBitonicASort::sort(int n, cl_mem in, cl_mem out) const
{
	int clStatus;
	int nk = 0;

	cl_mem buffers[2];
	buffers[0] = in;
	buffers[1] = out;
	int current = 0;


	for (int length = 1; length<n; length <<= 1) 
		for (int inc = length; inc>0; inc >>= 1)
		{
			//c->clearArgs(kid);
			clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffers[current]);
			clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffers[1 - current]);
			clSetKernelArg(kernel, 2, sizeof(int), &inc);
			int tmp = length << 1;
			clSetKernelArg(kernel, 3, sizeof(int), &tmp);

			//c->pushArg(kid, );
			//c->pushArg(kid, buffers[1 - current]);
			//c->pushArg(kid, length << 1);
			
			size_t global_size = n;
			size_t local_size = 256;
			//c->enqueueKernel(targetDevice, kid, n, 1, mWG, 1, EventVector());

			clStatus = clEnqueueNDRangeKernel(env.get_command_queue(), kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
			clEnqueueBarrier(env.get_command_queue());
			//c->enqueueKernel(targetDevice, kid, n, 1, 256, 1, EventVector());
			//c->enqueueBarrier(targetDevice); // sync
			current = 1 - current;
			nk++;
		}

	clFinish(env.get_command_queue());

	return (current == 1);  // output must be in OUT
}

double ParallelBitonicASort::memoryIO(int n) const
{
	double x = (double)n;
	double l = (double)log2(n);
	return (3 * (l*(l + 1)) / 2)*x*sizeof(data_t);
}

bool ParallelBitonicASort::checkOutput(int n, const data_t * in, const data_t * out) const
{
	return checkOutputFullSorted(n, in, out);
}

void ParallelBitonicASort::printID() const
{
	printf("Parallel bitonic A\n");
}