#pragma once
#include "OpenCLRuntimeEnv.h"
#include <CL/cl.h>
#include <string>

using std::string;


class ParallelBitonicLocalSort
{
public:
	ParallelBitonicLocalSort(const OpenCLRuntimeEnv &env, int wg, std::string src);
	virtual ~ParallelBitonicLocalSort();
	bool sort(int n, cl_mem in, cl_mem out) const;

private:
	const int mWG;
	const OpenCLRuntimeEnv &env;
	std::string src;
	cl_program program;
	cl_kernel kernel;

};

