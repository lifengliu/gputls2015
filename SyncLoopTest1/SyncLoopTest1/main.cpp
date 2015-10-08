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
#include <Windows.h>

double getRealTime()
{
	LARGE_INTEGER freq, value;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&value);
	return (double)value.QuadPart / (double)freq.QuadPart;
}

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



// Test a sorting algorithm
/*bool testSortingAlgorithm(int maxN, const SortingAlgorithm & algo, OpenCLRuntimeEnv& env1)
{
	// Setup OpenCL
	cl_context clContext = env1.get_context();
	
	if (clContext == 0) { printf("Context creation failed\n"); exit(1); }
	std::string errorMsg;
	char options[2000];
	//std::string algoOptions;
	//algo.getOptions(algoOptions);
	//snprintf(options, 2000, "-cl-fast-relaxed-math -D CONFIG_USE_VALUE=%d %s ", CONFIG_USE_VALUE, algoOptions.c_str());

	string s = loadFile("SortKernels.cl");
	//std::cout << s << std::endl;
	Context * c = Context::create(clContext, s.c_str(), options, KernelNames, errorMsg);
	clReleaseContext(clContext);
	if (c == 0) { printf("%s\n", errorMsg.c_str()); exit(1); }
	int targetDevice = c->getNDevices() - 1; // Run on last available device (assuming the X server is running on the first device)
											 // printf("Initialization OK [%s] targetDevice=%d\n",options,targetDevice);
	printf("____________________________________________________________\n");
#if CONFIG_USE_VALUE
	printf("Key+Value / ");
#else
	printf("Key / ");
#endif
	algo.printID();
	
	// Setup test vector
	data_t * a = new data_t[maxN];
	data_t * b = new data_t[maxN];
	for (int i = 0; i<maxN; i++)
	{
#if 1
		cl_uint x = (cl_uint)0;
		x = (x << 14) | ((cl_uint)rand() & 0x3FFF);
		x = (x << 14) | ((cl_uint)rand() & 0x3FFF);
#else
		cl_uint x = (cl_uint)(maxN - i);
#endif
		setKey(a[i], x);
		setValue(a[i], (cl_uint)i);
	}

	bool ok = true;
	for (int n = 256; n <= maxN && ok; n <<= 1)
	{
		// if (n < maxN) continue; // DEBUG

		// Test for N
		size_t sz = n * sizeof(data_t);
		cl_mem inBuffer = c->createBuffer(CL_MEM_READ_ONLY, sz, 0);
		cl_mem outBuffer = c->createBuffer(CL_MEM_READ_WRITE, sz, 0);
		Event e;
		e = c->enqueueWrite(targetDevice, inBuffer, true, 0, sz, a, EventVector()); // blocking
		c->finish(targetDevice);

		double t0 = getRealTime();
		double dt = 0;
		double nit = 0;
		for (int it = 1; it <= 1 << 20 && ok; it <<= 1)
		{
			for (int i = 0; i<it && ok; i++)
			{
				ok &= algo.sort(c, targetDevice, n, inBuffer, outBuffer);
				c->finish(targetDevice);
			}
			dt = getRealTime() - t0;
			nit += (double)it;
			if (dt > 0.5) break; // min time
		}
		if (!ok) { ok = true; continue; } // ignore launch errors
		dt /= nit;

		e = c->enqueueRead(targetDevice, outBuffer, true, 0, sz, b, EventVector()); // blocking
		double u = 1.0e-6 * (double)n / dt;
		printf("N=2^%d  R=%.2f Mkeys/s\n", log2(n), u);

		ok &= algo.checkOutput(n, a, b);
#if 0
		// Check debug output is all 0
		for (int i = 0; i<n; i++)
		{
			if (getKey(b[i]) != 0)
			{
				printf("Non-zero value, I=%d\n", i);
				for (int j = i - 10; j <= i + 10; j++)
				{
					if (j<0 || j >= n) continue;
					printf("OUT[%d] = %u\n", j, b[j]);
				}
				ok = false;
				break;
			}
		}
#endif
		clReleaseMemObject(inBuffer);
		clReleaseMemObject(outBuffer);

		if (dt > 4.0 || !ok) break; // Too long
	}

	delete[] a;
	delete[] b;
	delete c;

	return ok;
}*/

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
		clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_num[i]);
		printf("platform %u has %u devices\n", i, device_num[i]);

		plat_device_map[i] = new cl_device_id[device_num[i]];
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_num[i], plat_device_map[i], NULL);

		for (size_t j = 0; j < device_num[i]; j++) {
			OpenCLRuntimeEnv env;
			env.set_device(plat_device_map[i][j]);
			cl_context context = clCreateContext(NULL, 1, &plat_device_map[i][j], NULL, NULL, &clStatus);
			env.set_context(context);
			cl_command_queue_properties props[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
			cl_command_queue queue = clCreateCommandQueueWithProperties(context, plat_device_map[i][j], props, &clStatus);
			env.set_command_queue(queue);

			showDeviceInfo(plat_device_map[i][j]);
			
			int loopsize = 2048;
			int calcsize1 = 1024;
			int calcsize2 = 1024;

			int dn = 1024;
			data_t *d1 = new data_t[dn];
			data_t *d2 = new data_t[dn];
			
			for (int i = 0; i < dn; i++) {
				setKey(d1[i], dn - i + 50);
				setValue(d1[i], i);
			}
			cl_mem in = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(data_t) * dn, d1, &clStatus);
			cl_mem out = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(data_t) * dn, NULL, &clStatus);

			ParallelBitonicASort psort;
			psort.sort(sortsrc, env, dn, in, out);
			clFinish(env.get_command_queue());
			clStatus = clEnqueueReadBuffer(env.get_command_queue(), out, CL_TRUE, 0, sizeof(data_t) * dn, d2, 0, NULL, NULL);

			for (int i = 0; i < 1024; i++) {
				printf("%ud %ud\n", getKey(d2[i]), getValue(d2[i]));
			}
			delete[] d1;
			delete[] d2;
			clReleaseMemObject(in);
			clReleaseMemObject(out);


			/*SyncLoopExample sle(env, s, loopsize, calcsize1, calcsize2);
			
			//sle.sequentialCPU();
			sle.unremappedGPU();

			// --------------------
			sle.dependencyChecking();
			sle.evaluateBranch();
			sle.remappedGPU();
			
			auto m1 = sle.getTimer();
			long long totalTimeOurs = 0;

			fprintf(f, "%d\t%d\t%I64d\t%I64d\t%I64d\t%I64d\t%I64d\n", loopsize, j + 1, m1["dc"], m1["evaluateBranchGPU"],
				m1["sort"], m1["remappedLoopGPU"], m1["unremappedGPU"]
				);
			*/


			/*for (auto it = m1.begin(); it != m1.end(); ++it)
			{
				const string& key = it->first;
				long long value = it->second;
				fprintf(f, "%s\t%I64d\n", key.c_str(), value);
			}

			puts(""); */
			// ---------------------
			
			/*PGExample pge(env, s1, loopsize, calcsize1, calcsize2);
			pge.specExecute();
			pge.dependencyChecking();
			auto m2 = pge.getTimer();
			fprintf(f, "%I64d\t%I64d\n", m2["dc"], m2["specloopexec"]);
			*/


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

