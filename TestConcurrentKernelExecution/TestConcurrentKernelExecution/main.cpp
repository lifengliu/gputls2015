/*
* main.cpp
*
*  Created on: Sep 13, 2015
*      Author: hyliu
*/

#include <cstdio>
#include <algorithm>
#include <cstring>
#include <CL/cl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <vector>
#pragma warning(disable : 4996);

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::random_shuffle;
using std::vector;
using std::sort;
bool DEBUG = true;

void showDeviceInfo(const cl_device_id clid) {
	char *name_buf = new char[1280];
	char *vendor_buf = new char[1280];
	char *ext_buf = new char[8000];
	memset(name_buf, 0, 1280 * sizeof(char));
	memset(vendor_buf, 0, 1280 * sizeof(char));
	memset(ext_buf, 0, 8000 * sizeof(char));

	long long global_mem;
	size_t max_workgroup_size;
	char cl_profile_type[15];
	cl_device_type device_type;

	clGetDeviceInfo(clid, CL_DEVICE_NAME, sizeof(char) * 1280, name_buf, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_VENDOR, sizeof(char) * 1280, vendor_buf, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_EXTENSIONS, sizeof(char) * 8000, ext_buf, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem), &global_mem, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_workgroup_size), &max_workgroup_size, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_PROFILE, sizeof(char) * 14, cl_profile_type, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);

	printf("Using OpenCL device: %s %s\n with\n %s \nextension\n", vendor_buf, name_buf, ext_buf);
	printf("global memory size %lld %.2fGB\n", global_mem, global_mem / 1024.0 / 1024.0 / 1024.0);
	printf("max_workgroup_size = %u\n", max_workgroup_size);
	printf("profile type %s\n", cl_profile_type);
	printf("device type %d\n", device_type);
	

	delete[] name_buf;
	delete[] vendor_buf;
	delete[] ext_buf;
}

string loadFile(const string& fileLoc) {
	std::ifstream t1(fileLoc);
	string str((std::istreambuf_iterator<char>(t1)), std::istreambuf_iterator<char>());
	return str;
}

void testConcurrentKernel(cl_device_id cid) {
	int n_queue = 10;
	int clStatus;
	cl_context context = clCreateContext(NULL, 1, &cid, NULL, NULL, &clStatus);
	//cout << loadFile("test.cl") << endl;

	string program_str = loadFile("test.cl");
	const char *program_source = program_str.c_str();
	//printf("\n\n\n\n%s\n\n\n\n\n", program_str.c_str());
	//printf("\n\n\n\n%s\n\n\n\n\n", program_source);

	cl_program program = clCreateProgramWithSource(context, 1, &program_source, NULL, &clStatus);
	clStatus = clBuildProgram(program, 1, &cid, NULL, NULL, NULL);

	if (clStatus != CL_SUCCESS) {
		printf("clStatus = %d\n", clStatus); // -11 CL_BUILD_PROGRAM_FAILURE
		puts("build program error");
		size_t len;
		clGetProgramBuildInfo(program, cid, CL_PROGRAM_BUILD_LOG, NULL, NULL, &len);
		char *log = new char[len];
		memset(log, 0, sizeof(char) * len);
		clGetProgramBuildInfo(program, cid, CL_PROGRAM_BUILD_LOG, len, log, NULL);
		printf("%s\n", log);
		delete[] log;
		return;
	}


	unsigned int num_of_kernels;
	clStatus = clCreateKernelsInProgram(program, 0, NULL, &num_of_kernels);
	cl_kernel *kernels = new cl_kernel[num_of_kernels];
	clStatus = clCreateKernelsInProgram(program, num_of_kernels, kernels, NULL);

	if (DEBUG) {
		printf("num of kernels = %d\n", num_of_kernels);
		for (int i = 0; i < num_of_kernels; i++) {
			char kernel_name[50];
			clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, sizeof(kernel_name), kernel_name, NULL);
			printf("kernel %d %s\n", i, kernel_name);
		}

	}

	cl_command_queue *queues = new cl_command_queue[n_queue];

	cl_command_queue_properties props[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };

	for (int i = 0; i < n_queue; i++) {
		//queues[i] = clCreateCommandQueueWithProperties(context, cid, props, &clStatus);
		queues[i] = clCreateCommandQueue(context, cid, CL_QUEUE_PROFILING_ENABLE, &clStatus);
	}

	cl_event ev0 = clCreateUserEvent(context, &clStatus);

	cout << "user event" << clStatus << endl;


	cl_event *kernel_events = new cl_event[n_queue];
	
	int N = 50000;
	int calcSize = 100;

	cl_mem *dev_arr1 = new cl_mem[n_queue];
	cl_mem *dev_out = new cl_mem[n_queue];

	int *host_arr1 = new int[N];
	int *host_out = new int[N];
	memset(host_arr1, 0, sizeof(int) * N);
	memset(host_out, 0, sizeof(int) * N);

	for (int i = 0; i < N; i++) {
		host_arr1[i] = i % 7;
	}


	for (int i = 0; i < n_queue; i++) {
		int j = i % num_of_kernels;
		dev_arr1[i] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * N, host_arr1, &clStatus);
		//cout << "gege" << clStatus << endl;
		dev_out[i] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * N, host_out, &clStatus);

		//cout << "gege" << clStatus << endl;

		clSetKernelArg(kernels[j], 0, sizeof(cl_mem), &dev_arr1[i]);
		clSetKernelArg(kernels[j], 1, sizeof(cl_mem), &dev_out[i]);
		clSetKernelArg(kernels[j], 2, sizeof(cl_int), &N);
		clSetKernelArg(kernels[j], 3, sizeof(cl_int), &calcSize);

		size_t global_size = N;
		size_t local_size = 128;
		clStatus = clEnqueueNDRangeKernel(queues[i], kernels[j], 1, NULL, &global_size, NULL, 1, &ev0, &kernel_events[i]);
		//cout << "enqueue kernel" << clStatus << endl; 
	}

	clSetUserEventStatus(ev0, CL_COMPLETE);

	clWaitForEvents(n_queue, kernel_events);


	vector<std::pair<cl_ulong, cl_ulong>> v1;

	cl_ulong minstart = 0x7fffffff * 1000000l;
	for (int i = 0; i < n_queue; i++) {
		cl_ulong start = 0;
		cl_ulong end = 0;

		clStatus = clGetEventProfilingInfo(kernel_events[i], CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
		clStatus = clGetEventProfilingInfo(kernel_events[i], CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);
		//cout << "getprofile" << clStatus << endl;
		//	cout << "Event " << i << " start = " << start - 101145797001574 << " end = " << end - 101145797001574 << endl;
		v1.push_back(std::make_pair(start, end));
		if (start < minstart) {
			minstart = start;
		}
	}

	for (int i = 0; i < v1.size(); i++) {
		cl_ulong start = v1[i].first;
		cl_ulong end = v1[i].second;
		cout << "Event " << i << " start = " << start - minstart << " end = " << end - minstart << endl;
	}
	
	for (int i = 0; i < n_queue; i++) {
		clReleaseMemObject(dev_arr1[i]);
		clReleaseMemObject(dev_out[i]);
		clReleaseEvent(kernel_events[i]);
	}

	clReleaseEvent(ev0);

	for (int i = 0; i < n_queue; i++) {
		clReleaseCommandQueue(queues[i]);
	}

	delete[] kernel_events;
	delete[] dev_arr1;
	delete[] dev_out;
	delete[] queues;

	clReleaseContext(context);

	delete[] host_arr1;
	delete[] host_out;
	
}


int main(int argc, char *argv[]) {

	unsigned int num_platforms;
	int clStatus = clGetPlatformIDs(0, NULL, &num_platforms);

	printf("platform num = %d\n", num_platforms);

	cl_platform_id *platforms = new cl_platform_id[num_platforms];

	unsigned int *device_num = new unsigned int[num_platforms];

	clStatus = clGetPlatformIDs(num_platforms, platforms, NULL);

	cl_device_id **plat_device_map = new cl_device_id*[num_platforms];

	for (size_t i = 0; i < num_platforms; i++) {
		clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_num[i]);
		printf("platform %u has %u devices\n", i, device_num[i]);
		
		plat_device_map[i] = new cl_device_id[device_num[i]];
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_num[i], plat_device_map[i], NULL);

		for (size_t j = 0; j < device_num[i]; j++) {
			//printf("platform id = %d ,device %d\n", i, j);
			showDeviceInfo(plat_device_map[i][j]);
			testConcurrentKernel(plat_device_map[i][j]);
		}

	}


	delete[] platforms;
	delete[] device_num;

	for (size_t i = 0; i < num_platforms; i++) {
		delete[] plat_device_map[i];
	}

	delete[] plat_device_map;

	//cout << loadFile("test.cl") << endl;

	return 0;
}

