#ifndef GPUTLS001_UTILS_H_
#define GPUTLS001_UTILS_H_

#include <CL/opencl.h>
#include <memory>
#include <vector>
#include <cstring>

namespace gputls {
extern int getPlatformNum();
extern cl_platform_id *getPlatforms();
extern int getDeviceNum(cl_platform_id platform, int type = CL_DEVICE_TYPE_ALL);
extern cl_device_id *getDevices(cl_platform_id platform, int type = CL_DEVICE_TYPE_ALL);
extern cl_device_id getOneGPUDevice(int num);
extern cl_device_id getOneCPUDevice();
extern cl_kernel selectKernel(const char *kernelName, cl_kernel *kernels, int size);
extern char *loadFile(const char *fileName, int *fileSize);



}

namespace printfunc {
extern void display_device(cl_device_id device);
extern void printPlatformAndDevices();
extern void printKernelInfo(cl_kernel k);
extern void printExtensions();

}

#endif /* GPUTLS001_UTILS_H_ */

