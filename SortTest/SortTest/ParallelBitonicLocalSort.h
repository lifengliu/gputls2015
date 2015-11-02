#pragma once
#include <string>
#include "OpenCLRuntimeEnv.h"

using std::string;

class ParallelBitonicLocalSort {

public:
	ParallelBitonicLocalSort(const OpenCLRuntimeEnv & env, int wg, std::string src);
	bool sort(int n, cl_mem in, cl_mem out, int times) const;
	virtual ~ParallelBitonicLocalSort();
private:
	mutable int mLastN;

	const int wg;

	const OpenCLRuntimeEnv &env;
	std::string src;
	cl_program program;

	cl_kernel kernel;
};