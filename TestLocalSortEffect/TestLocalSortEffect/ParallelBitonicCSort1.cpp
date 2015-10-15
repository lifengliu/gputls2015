#include "ParallelBitonicCSort.h"
#include "data_t.h"
#include <algorithm>
#include <list>
#pragma warning(disable : 4996)

#define ALLOWB (2+4+8)

using std::list;
using std::min;

ParallelBitonicCSort::ParallelBitonicCSort(const OpenCLRuntimeEnv & env, int wg, std::string src):
	env(env), 
	wg(wg)
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
	}

	PARALLEL_BITONIC_C4_KERNEL = clCreateKernel(program, "ParallelBitonic_C4", &clStatus);
	PARALLEL_BITONIC_B16_KERNEL = clCreateKernel(program, "ParallelBitonic_B16", &clStatus);
	PARALLEL_BITONIC_B8_KERNEL = clCreateKernel(program, "ParallelBitonic_B8", &clStatus);
	PARALLEL_BITONIC_B4_KERNEL = clCreateKernel(program, "ParallelBitonic_B4", &clStatus);
	PARALLEL_BITONIC_B2_KERNEL = clCreateKernel(program, "ParallelBitonic_B2", &clStatus);

}

bool ParallelBitonicCSort::sort(int n, cl_mem in, cl_mem out) const
{
	clEnqueueCopyBuffer(env.get_command_queue(), in, out, 0, 0, n * sizeof(data_t), 0, NULL, NULL);
	clEnqueueBarrier(env.get_command_queue());

	cl_mem buffers[2];
	buffers[0] = in;
	buffers[1] = out;

	for (int length = 1; length < n; length <<= 1)
	{
		int inc = length;
		std::list<int> strategy; // vector defining the sequence of reductions
		{
			int ii = inc;
			while (ii>0)
			{
				if (ii == 128 || ii == 32 || ii == 8) { strategy.push_back(-1); break; } // C kernel
				int d = 1; // default is 1 bit
				if (0) d = 1;
#if 1
				// Force jump to 128
				else if (ii == 256) d = 1;
				else if (ii == 512 && (ALLOWB & 4)) d = 2;
				else if (ii == 1024 && (ALLOWB & 8)) d = 3;
				else if (ii == 2048 && (ALLOWB & 16)) d = 4;
#endif
				else if (ii >= 8 && (ALLOWB & 16)) d = 4;
				else if (ii >= 4 && (ALLOWB & 8)) d = 3;
				else if (ii >= 2 && (ALLOWB & 4)) d = 2;
				else d = 1;
				strategy.push_back(d);
				ii >>= d;
			}
		}

		while (inc > 0)
		{
			cl_kernel kernel;

			int ninc = 0;
			int doLocal = 0;
			int nThreads = 0;
			int d = strategy.front(); strategy.pop_front();

			switch (d)
			{
			case -1:
				kernel = PARALLEL_BITONIC_C4_KERNEL;
				ninc = -1; // reduce all bits
				doLocal = 4;
				nThreads = n >> 2;
				break;
			case 4:
				kernel = PARALLEL_BITONIC_B16_KERNEL;
				ninc = 4;
				nThreads = n >> ninc;
				break;
			case 3:
				kernel = PARALLEL_BITONIC_B8_KERNEL;
				ninc = 3;
				nThreads = n >> ninc;
				break;
			case 2:
				kernel = PARALLEL_BITONIC_B4_KERNEL;
				ninc = 2;
				nThreads = n >> ninc;
				break;
			case 1:
				kernel = PARALLEL_BITONIC_B2_KERNEL;
				ninc = 1;
				nThreads = n >> ninc;
				break;
			default:
				printf("Strategy error!\n");
				break;
			}
			
			int wg1;
			wg1 = std::min(wg, 256);
			wg1 = std::min(wg, nThreads);

			clSetKernelArg(kernel, 0, sizeof(cl_mem), &out);
			clSetKernelArg(kernel, 1, sizeof(int), &inc);   // INC passed to kernel
			int tmp = length << 1;
			clSetKernelArg(kernel, 2, sizeof(int), &tmp);   // DIR passed to kernel
			if (doLocal > 0) {
				clSetKernelArg(kernel, 3, sizeof(data_t) * doLocal * wg, NULL);   // DOLOCAL values / thread
			}


			size_t global_size = nThreads;
			size_t local_size = wg;

			clEnqueueNDRangeKernel(env.get_command_queue(), kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
			clEnqueueBarrier(env.get_command_queue());
			
											 // if (mLastN != n) printf("LENGTH=%d INC=%d KID=%d\n",length,inc,kid); // DEBUG
			if (ninc < 0) break; // done
			inc >>= ninc;
		}
	}
	clFinish(env.get_command_queue());
	mLastN = n;
	return true;
}


ParallelBitonicCSort::~ParallelBitonicCSort()
{
	clReleaseKernel(PARALLEL_BITONIC_C4_KERNEL);
	clReleaseKernel(PARALLEL_BITONIC_B16_KERNEL);
	clReleaseKernel(PARALLEL_BITONIC_B8_KERNEL);
	clReleaseKernel(PARALLEL_BITONIC_B4_KERNEL);
	clReleaseKernel(PARALLEL_BITONIC_B2_KERNEL);
	clReleaseProgram(program);
}



