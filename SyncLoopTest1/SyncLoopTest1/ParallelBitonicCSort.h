#pragma once
#include <string>
#include "OpenCLRuntimeEnv.h"

using std::string;

class ParallelBitonicCSort {

public:
	ParallelBitonicCSort(const OpenCLRuntimeEnv & env, int wg, std::string src);
	bool sort(int n, cl_mem in, cl_mem out) const;
	virtual ~ParallelBitonicCSort();
private:
	mutable int mLastN;

	const int wg;

	const OpenCLRuntimeEnv &env;
	std::string src;
	cl_program program;

	cl_kernel PARALLEL_BITONIC_C4_KERNEL,
		PARALLEL_BITONIC_B16_KERNEL,
		PARALLEL_BITONIC_B8_KERNEL,
		PARALLEL_BITONIC_B4_KERNEL,
		PARALLEL_BITONIC_B2_KERNEL;
};