#pragma once

#define NODE int
#define SHARED_ARRAY_ELEMENT_TYPE NODE
#define UNSHARED_ARRAY_ELEMENT_TYPE int
#define DUMB_COMPUTATION_NUM 10000

#include <CL/cl.h>

#include <string>
#include "OpenCLRuntimeEnv.h"
#include <map>

using std::string;
using std::map;


class TestLoopC
{
public:
	TestLoopC(const OpenCLRuntimeEnv& env, string kernelSourceCode, int numNodes, int numEdges, int ARRAY_SIZE);
	~TestLoopC();

	//follows Chenggang's legacy code...
	void init_XY_arrays();
	void init_Left_Right_Node_array();
	void init_application_data();
	void fill_data_to_gpu();
	void assign_GPU_memory();
	void release_GPU_memory();
	void init_program();
	
	void our_dc();
	void our_execute();

	void pg_spec();
	
	void prepare_kernels();


	int force_CPU(int Xi, int Xj);

	const map<string, long long> &getTimer() const;


private:
	int NumNodes;
	int NumEdges;
	int ARRAY_SIZE;


	NODE *X; // [NumNodes];
	NODE *Y; // [ARRAY_SIZE];

	NODE *LeftNode; // [NumEdges];
	NODE *RightNode; // [NumEdges];

	cl_mem d_X; 
	cl_mem d_Y;	//int* d_Y; 
	cl_mem d_leftNode; //	int* d_LeftNode; 
	cl_mem d_RightNode;	//int* d_RightNode; 

	cl_mem d_buffer;

	cl_mem d_read_trace_Y;
	cl_mem d_write_trace_Y;

	int raceFlag;
	cl_mem d_raceFlag;


	//NODE* Y_sequential_result;

	cl_kernel ourdc, ourexec, pgspec, pgdc1, pgreduce, pgdc3;

	cl_program program;
	
	const OpenCLRuntimeEnv &env;

	string src;

	map<string, long long> timer;
};

