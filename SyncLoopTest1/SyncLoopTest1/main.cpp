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

#include "SyncLoopExample.h"
#include "PGExample.h"
#include <map>

#include "SortingAlgorithm.h"
#include "ParallelBitonicLocalSort.h"
#include "ParallelBitonicASort.h"

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

void printMapKeys(const map<string, long long> m, FILE *f) {
	for (auto it = m.begin(); it != m.end(); ++it) {
		const string& key = it->first;
		fprintf(f, "\t%s", key.c_str());
	}
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

	string s = loadFile("clkernel.cl");
	string s1 = loadFile("pgspec.cl");
	string sortsrc = loadFile("SortKernels.cl");

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
			
			int calcsize1 = 1024 ;
			int calcsize2 = 1024 ;
			
			int startloop = 256;
			int endloop = 1048576 * 2;
			
			for (int loopsize = startloop; loopsize <= endloop; loopsize *= 2) {
				SyncLoopExample sle(env, s, loopsize, calcsize1, calcsize2);
				
				sle.init0();
				sle.sequentialCPU();
				sle.destroy0();

				// --------------------
				sle.init0();
				sle.dependencyChecking();
				sle.unremappedGPU();
				sle.destroy0();

				sle.init0();
				sle.dependencyChecking();
				sle.evaluateBranch();
				sle.partialSort(128);
				sle.remappedGPU();  
				sle.destroy0();

				// --------------------

				// --------------------
				sle.init0();
				sle.dependencyChecking();
				sle.evaluateBranch();
				sle.partialSort(256);
				sle.remappedGPU();
				sle.destroy0();
				// --------------------


				sle.init0();
				sle.dependencyChecking();
				sle.evaluateBranch();
				sle.fullySort();
				sle.remappedGPU();  
				sle.destroy0();

				// ---------------------

				PGExample pge(env, s1, loopsize, calcsize1, calcsize2);
				pge.specExecute();
				pge.dependencyChecking();
				
				//fprintf(f, "%I64d\t%I64d\n", m2["dc"], m2["specloopexec"]);



				auto m1 = sle.getTimer();
				auto m2 = pge.getTimer();

				m1.insert(m2.begin(), m2.end());
				
				if (loopsize == startloop) {
					fprintf(f, "%s\t%s", "loopsize", "device");
					printMapKeys(m1, f);
					fprintf(f, "\n");
				}

				fprintf(f, "%d\t%d", loopsize, j + 1);

				for (auto it = m1.begin(); it != m1.end(); ++it) {
					//const string& key = it->first;
					long long v = it->second;
					//fprintf(f, "\t%s", key.c_str());
					fprintf(f, "\t%I64d", v);
				}

				fprintf(f, "\n");



				/*fprintf(f, "%d\t%d\t%I64d\t%I64d\t%I64d\t%I64d\t%I64d\t%I64d\n", loopsize, j + 1, m1["dc"], m1["evaluateBranchGPU"],
					m1["partialSort"], m1["remappedLoopGPU"], m1["unremappedGPU"], 0ll
				);*/

			}

			
			/*for (int loopsize = 256; loopsize <= 1048576 * 2; loopsize *= 2) {
			
				PGExample pge(env, s1, loopsize, calcsize1, calcsize2);
				pge.specExecute();
				pge.dependencyChecking();
				auto m2 = pge.getTimer();
				fprintf(f, "%I64d\t%I64d\n", m2["dc"], m2["specloopexec"]);
			
			}*/

			/*auto m2 = pge.getTimer();
			long long totalTimePG = 0;
			for (auto it = m2.begin(); it != m2.end(); ++it)
			{
				const string& key = it->first;
				long long value = it->second;
				totalTimePG += value;
				fprintf(f, "%I64d\t", value);
			}*/
			
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

