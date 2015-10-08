#include "ParallelBitonicLocalSort.h"
#include "sortcommon.h"

#include <cstdio>

bool ParallelBitonicLocalSort::sort(std::string src, OpenCLRuntimeEnv& env, int n, cl_mem in, cl_mem out) const
{
	if ((n % mWG) != 0) return false; // Invalid

	int clStatus;
	cl_program program;

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
		return false;
	}


	cl_kernel kernel = clCreateKernel(program, "ParallelBitonic_Local", &clStatus);
		
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &in);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &out);
	clSetKernelArg(kernel, 2, sizeof(data_t) * mWG, NULL);

	int globalsize;
	int localsize;

	size_t global_size = n;
	size_t local_size = mWG;
	//c->enqueueKernel(targetDevice, kid, n, 1, mWG, 1, EventVector());

	clStatus = clEnqueueNDRangeKernel(env.get_command_queue(), kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	clFinish(env.get_command_queue());

	if (clStatus != 0) {
		puts("fuck");
	}


	clReleaseKernel(kernel);
	clReleaseProgram(program);
	
	return true;
}

double ParallelBitonicLocalSort::memoryIO(int n) const
{
	double x = (double)n; return (x*(double)mWG + 2 * x)*sizeof(data_t);
}

bool ParallelBitonicLocalSort::checkOutput(int n, const data_t * in, const data_t * out) const
{
	return checkOutputBlockSorted(n, mWG, in, out);
}

void ParallelBitonicLocalSort::printID() const
{
	printf("Parallel bitonic, local, WG=%d\n", mWG);
}