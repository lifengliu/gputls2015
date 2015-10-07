#pragma once

#include "OpenCLRuntimeEnv.h"
#include <string>
#include <map>
using std::map;
using std::string;

class PGExample
{
public:
	PGExample(const OpenCLRuntimeEnv& env, string kernelSourceCode, int loopsize, int calcSize1, int calcSize2);
	virtual ~PGExample();

	void specExecute();
	void dependencyChecking();

	const map<string, long long> getTimer() const;

private:
	
	map<string, long long> timer;
	const int loopsize;
	const int calcSize1;
	const int calcSize2;
	//host arrays
	int *host_c;
	int *host_a;
	int *host_P;
	int *host_Q;

	int *host_read_to;
	int *host_write_to;
	int *host_write_count;

	int raceFlag;

	
	OpenCLRuntimeEnv env;

	cl_mem dev_c;
	cl_mem dev_a;
	cl_mem dev_P;
	cl_mem dev_Q;

	cl_mem device_read_trace_a, device_write_trace_a; //a

	cl_mem device_read_to;
	cl_mem device_write_to;
	cl_mem device_write_count;

	cl_mem dev_raceflag;
	
		
	string kernelSourceCode;

	cl_program program;
	cl_kernel loopSpecKernel;
	cl_kernel dc1Kernel, dc2reduceKernel, dc3Kernel;


	void assign_host_memory();
	void destroy_host_memory();
	void init_host_memory();


	void assign_device_memory();
	void destroy_device_memory();
	void init_opencl_resources();
	void release_opencl_resources();

	void dc1();
	void dc2();
	void dc3();

	int dc_reduce(cl_mem &reduced_array, int length);
};

