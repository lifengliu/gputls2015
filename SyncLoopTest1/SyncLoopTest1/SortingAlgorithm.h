#pragma once


#include <cstdio>
#include <algorithm>
#include "sortcommon.h"

#include "OpenCLRuntimeEnv.h"

class SortingAlgorithm
{
public:
	// Sort IN[N] into OUT[N] using the specified context and device
	// This function will be called between two clFinish, and timed.

	virtual bool sort(std::string src, OpenCLRuntimeEnv& env, int n, cl_mem in, cl_mem out) const = 0;
	
	// Get total memory I/O of the algorithm for size N (Load + Store bytes)
	virtual double memoryIO(int n) const = 0;
	
	// Get additional OpenCL compilation options
	virtual void getOptions(std::string & options) const;
	
	// Print algorithm identification on stdout
	virtual void printID() const;

	// Check output. The base implementation compares OUT[N] to the sorted IN[N]
	virtual bool checkOutput(int n, const data_t * in, const data_t * out) const;
	

protected:
	// Specific check functions

	// Check OUT[N] is IN[N] sorted
	bool checkOutputFullSorted(int n, const data_t * in, const data_t * out) const;
	
	// Check OUT[N] is IN[N] sorted by blocks of size WG
	bool checkOutputBlockSorted(int n, int wg, const data_t * in, const data_t * out) const;
	
};



