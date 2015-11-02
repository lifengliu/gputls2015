#pragma once
#include "OpenCLRuntimeEnv.h"
#include <string>

using std::string;

class Test1
{
public:
	Test1(const OpenCLRuntimeEnv& env, string kernelSourceCode, int arrSize);
	~Test1();

	void mem_prep();
	void exec();
	
	
private:
	const OpenCLRuntimeEnv &env;
	cl_program program;
	cl_kernel kernel;
	const int arrSize;
	int alignedArrSize;

	cl_mem zcbuf;
	int *h_array;
	int cachelinesize = 64;

};

