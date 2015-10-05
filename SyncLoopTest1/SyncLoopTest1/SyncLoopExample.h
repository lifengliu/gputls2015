#ifndef GPUTLS001_BEFORECHECKINGEXAMPLE2_H_
#define GPUTLS001_BEFORECHECKINGEXAMPLE2_H_

#include <CL/opencl.h>
#include "OpenCLRuntimeEnv.h"

#include <string>



/*


#pragma scop
for (int i = 0; i < N; i++) {
	if (c[i] > 0) {
		a[Q[i]] = comp1(size1) + a[P[i]];
	} else {
		a[Q[i]] = comp2(size2);
	}
}
#pragma endscop



*/


using std::string;


struct IndexNode;

class SyncLoopExample {
public:
	SyncLoopExample(const OpenCLRuntimeEnv& env, string kernelSourceCode, int loopsize, int calcSize1, int calcSize2);
	virtual ~SyncLoopExample();
	
	void sequentialCPU();
	void unremappedGPU();

	
private:
	const int loopsize;
	const int calcSize1;
	const int calcSize2;
	//host arrays
	int *host_c;
	int *host_a;
	int *host_P;
	int *host_Q;

	//end host arrays



	//device arrays
	cl_mem dev_c;
	cl_mem dev_a;
	cl_mem dev_P;
	cl_mem dev_Q;



	//end device arrays

	string kernelSourceCode;

	OpenCLRuntimeEnv env;
	cl_program program;
	cl_kernel loopKernelOrigin;

	void assign_host_memory();
	void destroy_host_memory();
	void init_opencl_resources();
	void init_host_memory();

	void assign_device_memory();
	void destroy_device_memory();
	void release_opencl_resources();
	
};


#endif /* GPUTLS001_BEFORECHECKINGEXAMPLE2_H_ */
