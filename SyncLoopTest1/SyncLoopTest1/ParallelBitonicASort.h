#pragma once
#include "SortingAlgorithm.h"
class ParallelBitonicASort : public SortingAlgorithm
{
public:
	ParallelBitonicASort(const OpenCLRuntimeEnv& env, std::string src);
	virtual ~ParallelBitonicASort();

	bool sort(int n, cl_mem in, cl_mem out) const;
	
	double memoryIO(int n) const;
	
	bool checkOutput(int n, const data_t * in, const data_t * out) const;
	void printID() const;

private:
	const OpenCLRuntimeEnv &env;
	std::string src;
	cl_program program;
	cl_kernel kernel;
};

