#pragma once
#include "SortingAlgorithm.h"
class ParallelBitonicASort : public SortingAlgorithm
{


public:
	ParallelBitonicASort() { }
	bool sort(std::string src, OpenCLRuntimeEnv& env, int n, cl_mem in, cl_mem out) const;
	
	double memoryIO(int n) const;
	
	bool checkOutput(int n, const data_t * in, const data_t * out) const;
	void printID() const;
};

