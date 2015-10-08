#pragma once
#include "SortingAlgorithm.h"
#include "sortcommon.h"

class ParallelBitonicLocalSort : public SortingAlgorithm
{
public:
	ParallelBitonicLocalSort(int wg) : mWG(wg) { }
	bool sort(std::string src, OpenCLRuntimeEnv& env, int n, cl_mem in, cl_mem out) const;

	double memoryIO(int n) const;
	bool checkOutput(int n, const data_t * in, const data_t * out) const;
	
	void printID() const;

private:
	int mWG;
};

