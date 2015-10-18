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
#include "OpenCLRuntimeEnv.h"
#include <map>
#include <chrono>
#include "data_t.h"
#include "ParallelBitonicCSort.h"

#pragma warning(disable : 4996)

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::random_shuffle;
using std::vector;
using std::sort;
using std::map;
using std::fill;
using std::map;
using std::make_pair;
using std::string;

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

	char dev_version[200];
	char dev_driver_version[200];

	memset(dev_version, 0, 200 * sizeof(char));
	memset(dev_driver_version, 0, 200 * sizeof(char));

	clGetDeviceInfo(clid, CL_DEVICE_VERSION, sizeof(char) * 200, dev_version, NULL);
	clGetDeviceInfo(clid, CL_DRIVER_VERSION, sizeof(char) * 200, dev_driver_version, NULL);

	printf("Using OpenCL device: %s %s\n with\n %s \nextension\n", vendor_buf, name_buf, ext_buf);
	printf("global memory size %lld %.2fGB\n", global_mem, global_mem / 1024.0 / 1024.0 / 1024.0);
	printf("max_workgroup_size = %u\n", max_workgroup_size);
	printf("profile type %s\n", cl_profile_type);
	printf("device type %d\n", device_type);
	printf("device version %s\n", dev_version);
	printf("device driver version %s\n", dev_driver_version);

	delete[] name_buf;
	delete[] vendor_buf;
	delete[] ext_buf;
}

string loadFile(const string& fileLoc) {
	std::ifstream t1(fileLoc);
	string str((std::istreambuf_iterator<char>(t1)), std::istreambuf_iterator<char>());
	return str;
}


int main(int argc, char *argv[]) {

	freopen("gege.txt", "w", stdout);
	auto f = fopen("gege1.txt", "w");

	unsigned int num_platforms;
	int clStatus = clGetPlatformIDs(0, NULL, &num_platforms);

	printf("platform num = %d\n", num_platforms);

	cl_platform_id *platforms = new cl_platform_id[num_platforms];

	unsigned int *device_num = new unsigned int[num_platforms];

	clStatus = clGetPlatformIDs(num_platforms, platforms, NULL);

	cl_device_id **plat_device_map = new cl_device_id*[num_platforms];

	string sortsrc = loadFile("SortKernels.cl");
	//printf("%s", sortsrc.c_str());
	string src = loadFile("gege.cl");

	for (size_t i = 0; i < num_platforms; i++) {
		clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &device_num[i]);
		printf("platform %u has %u devices\n", i, device_num[i]);

		plat_device_map[i] = new cl_device_id[device_num[i]];
		//CL_DEVICE_TYPE_ALL
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, device_num[i], plat_device_map[i], NULL);

		for (size_t j = 0; j < device_num[i]; j++) {
			OpenCLRuntimeEnv env;
			env.set_device(plat_device_map[i][j]);
			cl_context context = clCreateContext(NULL, 1, &plat_device_map[i][j], NULL, NULL, &clStatus);
			env.set_context(context);
			cl_command_queue_properties props[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
			cl_command_queue queue = clCreateCommandQueueWithProperties(context, plat_device_map[i][j], props, &clStatus);
			env.set_command_queue(queue);


			showDeviceInfo(plat_device_map[i][j]);


			int len = (1 << 12);

			for (int VD = 2; VD <= 16; VD++) {
				for (int SORT = 8; SORT <= 13; SORT++) {
					data_t *host_in = new data_t[len];
					data_t *host_out = new data_t[len];

					for (int k = 0; k < len; k++) {
						setKey(host_in[k], k % VD);
						setValue(host_in[k], k);
					}


					std::random_shuffle(host_in, host_in + len);

					cl_mem dev_in = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(data_t) * len, host_in, &clStatus);
					cl_mem dev_out = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(data_t) * len, NULL, &clStatus);

					string s = loadFile("sort.cl");

					ParallelBitonicCSort psort(env, 256, s);


					auto start = std::chrono::high_resolution_clock::now();
					psort.sort(len, dev_in, dev_out, 100);    // 8:  2 inc 2 dec     9:    4 inc 4 dec    10  8 inc  8 dec
					auto end = std::chrono::high_resolution_clock::now();
					auto cpu_elapsedtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
					std::cout << "partial sort = " << cpu_elapsedtime << "ns" << std::endl;

					clEnqueueReadBuffer(env.get_command_queue(), dev_out, CL_TRUE, 0, len * sizeof(data_t), host_in, 0, NULL, NULL);

					int badsum = 0;
					int C = 64;

					for (int u = 0; u < len; u += C) {
						bool flag = false;

						for (int v = u; v < std::min(u + C, len); v++) {
							if (v + 1 < std::min(u + C, len) && getKey(host_in[v]) != getKey(host_in[v + 1])) {
								flag = true;
							}
						}

						if (flag) {
							badsum++;
						}
					}


					/*for (int k = 0; k < len; k++) {
						if (k % 64 == 0) puts("---------------------------");
						printf("%d\n", getKey(host_in[k]));

					}*/

					printf("value domain = %d\n", VD);
					printf("sort part = %d\n", SORT);
					printf("badsum = %d\n", badsum);
					
					delete[] host_in;
					delete[] host_out;
					clReleaseMemObject(dev_in);
					clReleaseMemObject(dev_out);
				}
			}

		}
	}


	delete[] platforms;
	delete[] device_num;

	for (size_t i = 0; i < num_platforms; i++) {
		delete[] plat_device_map[i];
	}

	delete[] plat_device_map;

	return 0;
}
