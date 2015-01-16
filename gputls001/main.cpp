#include <cstdio>

#include <CL/opencl.h>

#include "gputlsconsts.h"

#include "utils.h"



typedef struct TraceNode {
    cl_int size;
    cl_int indices[TRACE_SIZE];  // record index, not address
    //for the case containing multiple arrays, maybe a code generator is needed.
} TraceNode;


int main (int argc, const char *argv[]) {

	printfunc::printPlatformAndDevices();
	printfunc::printExtensions();

	float *host_A = new float[NUM_VALUES];
	float *host_B = new float[NUM_VALUES];
	int *host_P = new int[NUM_VALUES];
	int *host_Q = new int[NUM_VALUES];

    for (int i = 0; i < NUM_VALUES; i++) {
        host_A[i] = i;
        host_P[i] = i;
        host_Q[i] = i;
    }

	cl_device_id device = gputls::getOneGPUDevice();
	cl_int clStatus;
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &clStatus);
	cl_command_queue command_queue = clCreateCommandQueue(context, device, 0, &clStatus);

	int sourceSize;
	char *clSourceCode = gputls::loadFile("gputls001/gputls.cl", &sourceSize);

	//printf("%s\n", clSourceCode);

	cl_program program = clCreateProgramWithSource(context, 1, (const char **) &clSourceCode, NULL, &clStatus);
	clStatus = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	if (clStatus != CL_SUCCESS) {
		printf("clStatus = %d\n", clStatus);
		puts("build Program Error");
		size_t len;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &len);
		char *log = new char[len];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len, log, NULL);
		printf("%s\n", log);
		return -1;
	}



	cl_kernel testTlsKernel = clCreateKernel(program, "test_kernel", &clStatus);

	cl_kernel DC1kernel = clCreateKernel(program, "dependency_checking_phase_one", &clStatus);
	cl_kernel reduceKernel = clCreateKernel(program, "reduce", &clStatus);
	cl_kernel DC3kernel = clCreateKernel(program, "dependency_checking_phase_three", &clStatus);



	if (clStatus != CL_SUCCESS) {
		puts("create kernel error");

	}

	cl_mem device_A = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(float), host_A, &clStatus);
	cl_mem device_B = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(float), host_B, &clStatus);
	cl_mem device_P = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(int), host_P, &clStatus);
	cl_mem device_Q = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(int), host_Q, &clStatus);

	cl_mem device_read_trace = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_VALUES * sizeof(TraceNode), NULL, &clStatus);
	cl_mem device_write_trace = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_VALUES * sizeof(TraceNode), NULL, &clStatus);
	cl_mem device_read_to = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_VALUES * sizeof(cl_int), NULL, &clStatus);
	cl_mem device_write_to = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_VALUES * sizeof(cl_int), NULL, &clStatus);
	cl_mem device_write_count = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_VALUES * sizeof(cl_int), NULL, &clStatus);
	cl_mem device_misspeculation = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_int), NULL, &clStatus);


	cl_mem device_reduce_writeTo = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_int), NULL, &clStatus);
	cl_mem device_reduce_writeCount = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_int), NULL, &clStatus);



	clSetKernelArg(testTlsKernel, 0, sizeof(cl_mem), &device_A);
	clSetKernelArg(testTlsKernel, 1, sizeof(cl_mem), &device_B);
	clSetKernelArg(testTlsKernel, 2, sizeof(cl_mem), &device_P);
	clSetKernelArg(testTlsKernel, 3, sizeof(cl_mem), &device_Q);
	clSetKernelArg(testTlsKernel, 4, sizeof(cl_mem), &device_read_trace);
	clSetKernelArg(testTlsKernel, 5, sizeof(cl_mem), &device_write_trace);

	size_t global_size = NUM_VALUES;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, testTlsKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	if (clStatus != CL_SUCCESS) {
		puts("create kernel error");

	}

	clEnqueueReadBuffer(command_queue, device_A, CL_TRUE, 0, NUM_VALUES * sizeof(float), host_A, 0, NULL, NULL);

	for (int i = 0; i < 5; i ++ ){
		printf("%.2f\n", host_A[i]);
	}

	puts("fuck here");

	clSetKernelArg(dependencyCheckingKernel, 0, sizeof(cl_mem), &device_read_trace);
	clSetKernelArg(dependencyCheckingKernel, 1, sizeof(cl_mem), &device_write_trace);
	clSetKernelArg(dependencyCheckingKernel, 2, sizeof(cl_mem), &device_read_to);
	clSetKernelArg(dependencyCheckingKernel, 3, sizeof(cl_mem), &device_write_to);
	clSetKernelArg(dependencyCheckingKernel, 4, sizeof(cl_mem), &device_write_count);
	clSetKernelArg(dependencyCheckingKernel, 5, sizeof(cl_mem), &device_misspeculation);

	clSetKernelArg(dependencyCheckingKernel, 6, sizeof(cl_int) * local_size, NULL);

	clStatus = clEnqueueNDRangeKernel(command_queue, dependencyCheckingKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	int spec = -1;
	clEnqueueReadBuffer(command_queue, device_misspeculation, CL_TRUE, 0, sizeof(cl_int), &spec, 0, NULL, NULL);


	clFlush(command_queue);
	clFinish(command_queue);




	printf("spec = %d\n", spec);

	delete[] host_A;
	delete[] host_B;
	delete[] host_Q;
	delete[] host_P;

	clReleaseContext(context);
	clReleaseCommandQueue(command_queue);
	clReleaseMemObject(device_A);
	clReleaseMemObject(device_B);
	clReleaseMemObject(device_P);
	clReleaseMemObject(device_Q);
	clReleaseMemObject(device_read_trace);
	clReleaseMemObject(device_write_trace);
	clReleaseMemObject(device_read_to);
	clReleaseMemObject(device_write_to);
	clReleaseMemObject(device_write_count);
	clReleaseMemObject(device_misspeculation);

 	return 0;
}



