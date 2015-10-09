#pragma once
#include "SortingAlgorithm.h"
#include "sortcommon.h"

class ParallelBitonicLocalSort : public SortingAlgorithm
{
public:
	ParallelBitonicLocalSort(const OpenCLRuntimeEnv &env, int wg, std::string src);
	virtual ~ParallelBitonicLocalSort();
	bool sort(int n, cl_mem in, cl_mem out) const;

	double memoryIO(int n) const;
	bool checkOutput(int n, const data_t * in, const data_t * out) const;
	
	void printID() const;

private:
	const int mWG;
	const OpenCLRuntimeEnv &env;
	std::string src;
	cl_program program;
	cl_kernel kernel;

};

