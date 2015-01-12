/*
 * utils.h
 *
 *  Created on: Jan 12, 2015
 *      Author: hyliu
 */

#ifndef GPUTLS001_UTILS_H_
#define GPUTLS001_UTILS_H_

#include <CL/opencl.h>
#include <memory>
#include <vector>

namespace gputls {
using std::auto_ptr;

int getPlatformNum() {
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

cl_platform_id *getPlatforms() {
	int platNum = getPlatformNum();
	cl_platform_id *platforms = new cl_platform_id[platNum];
	cl_int clStatus = clGetPlatformIDs(platNum, platforms, NULL);

	if (clStatus != CL_SUCCESS) {
		return NULL;
	}

	return platforms;
}

int getDeviceNum(cl_platform_id platform, int type = CL_DEVICE_TYPE_ALL) {
	cl_int clStatus;
	cl_uint num_devices;
	clStatus = clGetDeviceIDs(platform, type, 0, NULL, &num_devices);
	return (clStatus == CL_SUCCESS) ? num_devices : -1;
}

cl_device_id *getDevices(cl_platform_id platform,
		int type = CL_DEVICE_TYPE_ALL) {
	int deviceNum = getDeviceNum(platform);

	if (deviceNum == -1) {
		return NULL;
	}

	cl_device_id *devices = new cl_device_id[deviceNum];
	cl_int clStatus = clGetDeviceIDs(platform, type, deviceNum, devices, NULL);

	return devices;
}

}

namespace printfunc {

void display_device(cl_device_id device) {
	char name_buf[128];
	char vendor_buf[128];

	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(char) * 128, name_buf, NULL);
	clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(char) * 128, vendor_buf,
			NULL);

	fprintf(stdout, "Using OpenCL device: %s %s\n", vendor_buf, name_buf);

}
void printPlatformAndDevices() {
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


}

#endif /* GPUTLS001_UTILS_H_ */

