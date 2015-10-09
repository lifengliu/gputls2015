#ifndef GPUTLS001_BEFORECHECKINGEXAMPLE2_H_
#define GPUTLS001_BEFORECHECKINGEXAMPLE2_H_

#include <CL/opencl.h>
#include "OpenCLRuntimeEnv.h"

#include <string>

#include "sortcommon.h"

#include <map>
using std::map;

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


class SyncLoopExample {
public:
	SyncLoopExample(const OpenCLRuntimeEnv& env, string kernelSourceCode, int loopsize, int calcSize1, int calcSize2);
	virtual ~SyncLoopExample();
	
	void sequentialCPU();
	void unremappedGPU();
	void remappedGPU();
	void evaluateBranch();
	void dependencyChecking();
	const map<string, long long> &getTimer() const;

private:
	const int loopsize;
	const int calcSize1;
	const int calcSize2;
	//host arrays
	int *host_c;
	int *host_a;
	int *host_P;
	int *host_Q;
	data_t *host_indexnode;


	int raceFlag;
	//end host arrays



	//device arrays
	cl_mem dev_c;
	cl_mem dev_a;
	cl_mem dev_P;
	cl_mem dev_Q;
	cl_mem dev_indexnode, dev_indexnodeout;
	
	cl_mem dev_buffer;
	cl_mem dev_raceflag;

	//end device arrays

	string kernelSourceCode;

	OpenCLRuntimeEnv env;
	cl_program program;
	cl_kernel loopKernelOrigin;
	cl_kernel loopKernelRemapped;
	cl_kernel branchEvaluateKernel;
	cl_kernel dcwriteonaKernel;
	cl_kernel dcreadonaKernel;

	void assign_host_memory();
	void destroy_host_memory();
	void init_host_memory();
	

	void assign_device_memory();
	void destroy_device_memory();
	void init_opencl_resources();
	void release_opencl_resources();
	

	void dc_write_on_a();
	void dc_read_on_a();

	map<string, long long> timer;
};


#endif /* GPUTLS001_BEFORECHECKINGEXAMPLE2_H_ */
