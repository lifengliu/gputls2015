#include "Test1.h"
#include "OpenCLRuntimeEnv.h"
#include <string>

#include <malloc.h>
#include <algorithm>

Test1::Test1(const OpenCLRuntimeEnv& env, string kernelSourceCode, int arrSize) :
	env(env),
	arrSize(arrSize)
{
	cl_int clStatus;
	const char *program_source = kernelSourceCode.c_str();
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

	kernel = clCreateKernel(program, "test1", &clStatus);

	clGetDeviceInfo(env.get_device_id(), CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cachelinesize), &cachelinesize, NULL);

	

	alignedArrSize = cachelinesize * (1 + (arrSize - 1) / cachelinesize);
	printf("cachelinesize = %d  aignedarraysize = %d\n", cachelinesize, alignedArrSize);

	h_array = (int *) _aligned_malloc(alignedArrSize * sizeof(int), 4096);
	//clCreateBuffer(…CL_MEM_USE_HOST_PTR, arraySizeAligned, inputArray);

}


Test1::~Test1()
{
	clReleaseKernel(kernel);
	clReleaseProgram(program);

	_aligned_free(h_array);
}


void Test1::mem_prep()
{
	int clStatus;
	zcbuf = clCreateBuffer(env.get_context(), CL_MEM_USE_HOST_PTR, alignedArrSize * sizeof(int), h_array, &clStatus);

	//clCreateBuffer(env.get_context(), CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, arrSize * sizeof(int), NULL, &clStatus); // zero copy , read only in host

	int zero = 0;
	clEnqueueFillBuffer(env.get_command_queue(), zcbuf, &zero, sizeof(int), 0, sizeof(int) * arrSize, 0, NULL, NULL);

}

void Test1::exec()
{
	int clStatus;

	size_t global_work_size[1] = { arrSize };
	size_t block_size[1] = { 32 };

	clSetKernelArg(kernel, 0, sizeof(cl_mem),  &zcbuf);
	clSetKernelArg(kernel, 1, sizeof(int), &arrSize);

	clEnqueueNDRangeKernel(env.get_command_queue(), kernel, 1, NULL, global_work_size, block_size, 0, NULL, NULL);

	clFinish(env.get_command_queue());

	void *gege =  clEnqueueMapBuffer(env.get_command_queue(), zcbuf, CL_TRUE, CL_MAP_READ, 0, alignedArrSize * sizeof(int), 0, NULL, NULL, &clStatus);

	int *miaomiao = (int*)gege;

	long long sum = 0;
	for (int i = 0; i < std::min(arrSize, 1000); i++) {
		sum += miaomiao[i] ;
		sum = sum % 10000;
	}

	printf("%I64d\n", sum);

}






