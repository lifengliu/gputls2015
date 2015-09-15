/*
 * utils.cpp
 *
 *  Created on: Mar 10, 2015
 *      Author: hyliu
 */




#include "utils.h"
#include <CL/opencl.h>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdio>


int gputls::getPlatformNum() {
	//cl_platform_id *platforms = NULL;
	cl_uint num_platforms;

	cl_int clStatus = clGetPlatformIDs(0, NULL, &num_platforms);
	return (clStatus == CL_SUCCESS) ? num_platforms : -1;
}

/*auto_ptr<std::vector<cl_platform_id> > getPlatforms() {
 int platNum = getPlatformNum();
 cl_platform_id *platforms = new cl_platform_id[platNum];
 cl_int clStatus = clGetPlatformIDs(platNum, platforms, NULL);

 if (clStatus != CL_SUCCESS) {
 delete[] platforms;
 return NULL;
 }

 std::vector<cl_platform_id> plat_vec(platforms);

 return auto_ptr<std::vector<cl_platform_id> > (plat_vec);

 }*/

cl_platform_id *gputls::getPlatforms() {
	int platNum = getPlatformNum();
	cl_platform_id *platforms = new cl_platform_id[platNum];
	cl_int clStatus = clGetPlatformIDs(platNum, platforms, NULL);

	if (clStatus != CL_SUCCESS) {
		return NULL;
	}

	return platforms;
}

int gputls::getDeviceNum(cl_platform_id platform, int type = CL_DEVICE_TYPE_ALL) {
	cl_int clStatus;
	cl_uint num_devices;
	clStatus = clGetDeviceIDs(platform, type, 0, NULL, &num_devices);
	return (clStatus == CL_SUCCESS) ? num_devices : -1;
}

cl_device_id *gputls::getDevices(cl_platform_id platform, int type = CL_DEVICE_TYPE_ALL) {
	int deviceNum = getDeviceNum(platform);

	if (deviceNum == -1) {
		return NULL;
	}

	cl_device_id *devices = new cl_device_id[deviceNum];
	cl_int clStatus = clGetDeviceIDs(platform, type, deviceNum, devices, NULL);

	return clStatus == CL_SUCCESS ? devices : NULL;
}

cl_device_id gputls::getOneGPUDevice(int num) {
	cl_platform_id *plats = getPlatforms();
	cl_device_id *devices = getDevices(plats[0], CL_DEVICE_TYPE_GPU);
	delete[] plats;
	cl_device_id device_id_ret = devices[num];
	delete[] devices;
	return device_id_ret;
}

cl_device_id gputls::getOneCPUDevice() {
	cl_platform_id *plats = getPlatforms();
	cl_device_id *devices = getDevices(plats[0], CL_DEVICE_TYPE_CPU);
	delete[] plats;
	cl_device_id device_id_ret = devices[0];
	delete[] devices;
	return device_id_ret;
}


cl_kernel gputls::selectKernel(const char *kernelName, cl_kernel *kernels, int size) {
	for (int i = 0; i < size; i++) {
		char name[50];
		memset(name, 0, sizeof(name));
		clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, 50, name, NULL);
		if (strcmp(name, kernelName) == 0) {
			return kernels[i];
		}
	}

	return NULL;

}

char *gputls::loadFile(const char *fileName, int *fileSize) {

	FILE *fp;
	long lSize;
	char *buffer;

	fp = fopen(fileName, "rb");
	if (!fp)
		perror(fileName), exit(1);

	fseek(fp, 0L, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);

	if (fileSize != NULL) {
		*fileSize = lSize;
	}

	/* allocate memory for entire content */
	buffer = (char *) calloc(1, lSize + 1);
	if (!buffer)
		fclose(fp), fputs("memory alloc fails", stderr), exit(1);

	/* copy the file into the buffer */
	if (1 != fread(buffer, lSize, 1, fp))
		fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

	/* do your work here, buffer is a string contains the whole text */

	fclose(fp);

	return buffer;
	//free(buffer);
}




void printfunc::display_device(cl_device_id device) {
	char name_buf[128];
	char vendor_buf[128];
	char ext_buf[8000];

	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(char) * 128, name_buf, NULL);
	clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(char) * 128, vendor_buf, NULL);

	clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(char) * 8000, ext_buf, NULL);
	fprintf(stdout, "Using OpenCL device: %s %s\n with\n %s \nextension\n\n", vendor_buf, name_buf, ext_buf);

}

void printfunc::printPlatformAndDevices() {
	int platformNum = gputls::getPlatformNum();
	printf("platform num = %d\n", platformNum);
	cl_platform_id *plats = gputls::getPlatforms();

	for (int i = 0; i < platformNum; i++) {
		int plat_device_num = gputls::getDeviceNum(plats[i]);
		printf("platform %d has %d devices\n", i, plat_device_num);
		int plat_device_gpu_num = gputls::getDeviceNum(plats[i],
		CL_DEVICE_TYPE_GPU);
		printf("platform %d has %d gpu devices\n", i, plat_device_gpu_num);

		cl_device_id *devices = gputls::getDevices(plats[i]);

		for (int j = 0; j < plat_device_num; j++) {
			display_device(devices[j]);
		}

		delete[] devices;

	}

	delete[] plats;
}

void printfunc::printKernelInfo(cl_kernel k) {
	//clGetKernelInfo(k, CL_KERNEL_LOCAL_MEM_SIZE)
}

void printfunc::printExtensions() {
	cl_platform_id *platforms;
	cl_uint num_platforms;
	cl_int err;//
	//, platform_index = -1;

	char *ext_data;
	size_t ext_size;
	//const char icd_ext[] = "cl_khr_icd";

	err = clGetPlatformIDs(10, NULL, &num_platforms);

	if (err < 0) {
		puts("cound not find any platforms");
		return;
	}

	platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id) * num_platforms);

	clGetPlatformIDs(num_platforms, platforms, NULL);

	for (size_t i = 0; i < num_platforms; i++) {
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 0, NULL, &ext_size);
		if (err < 0 ) {
			puts("could not read extension data");
			free(platforms);
			return;
		}

		ext_data = (char *) malloc(ext_size * sizeof(char));

		clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, ext_size, ext_data, NULL);

		printf("platform %d supports extensions %s\n", i, ext_data);

		free(ext_data);
	}


	free(platforms);
}

