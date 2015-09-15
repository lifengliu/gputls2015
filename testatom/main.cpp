/*
 * main.cpp
 *
 *  Created on: Sep 13, 2015
 *      Author: hyliu
 */

#include <cstdio>
#include <CL/opencl.h>
#include <algorithm>
#include <time.h>

using std::random_shuffle;

char *loadFile(const char *fileName, int *fileSize) {
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



void showDeviceInfo(cl_device_id clid) {
	char name_buf[128];
	char vendor_buf[128];
	char ext_buf[8000];
	cl_ulong global_mem;
	size_t max_workgroup_size;
	char cl_profile_type[15];
	cl_device_type device_type;

	clGetDeviceInfo(clid, CL_DEVICE_NAME, sizeof(char) * 128, name_buf, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_VENDOR, sizeof(char) * 128, vendor_buf, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_EXTENSIONS, sizeof(char) * 8000, ext_buf, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem), &global_mem, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_workgroup_size), &max_workgroup_size, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_PROFILE, sizeof(char) * 15, cl_profile_type, NULL);
	clGetDeviceInfo(clid, CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);

	printf("Using OpenCL device: %s %s\n with\n %s \nextension\n", vendor_buf, name_buf, ext_buf);
	printf("global memory size %ul %.2fGB\n", global_mem, global_mem / 1024.0 / 1024.0 / 1024.0);
	printf("max_workgroup_size = %u\n", max_workgroup_size);
	printf("profile type %s\n", cl_profile_type);

	printf("device type %d\n", device_type);

	//CL_DEVICE_TYPE_CPU
	//CL_DEVICE_TYPE_GPU
	//CL_DEVICE_TYPE_ACCELERATOR
	//CL_DEVICE_TYPE_DEFAULT

}

void printTitle() {
	printf("SIZE\tqueued_to_submit_time\tsubmit_to_start_time\tstart_to_end_time\ttotal_profile_time\tall_time");
	printf("\tcopy_time\n");

}

void atomtest(cl_device_id exec_device, int SIZE) {
	int fileSize;
	char *kernel_program_string = loadFile("atomtest.cl", &fileSize);

	int clStatus;
	cl_context context = clCreateContext(NULL, 1, &exec_device, NULL, NULL, &clStatus);
	cl_command_queue command_queue = clCreateCommandQueue(context, exec_device, CL_QUEUE_PROFILING_ENABLE, &clStatus);
	cl_program program = clCreateProgramWithSource(context, 1, (const char **) &kernel_program_string, NULL, &clStatus);
	clStatus = clBuildProgram(program, 1, &exec_device, NULL, NULL, NULL);

	cl_uint kernel_num;
	clStatus = clCreateKernelsInProgram(program, 0, NULL, &kernel_num);

	cl_kernel kernel = clCreateKernel(program, "atom_test", &clStatus);

	int *host_idx_array = new int[SIZE];
	int *host_buf_array = new int[SIZE];
	int *host_res_buf_array = new int[SIZE];

	for (int i = 0; i < SIZE; i++) {
		host_buf_array[i] = 0;
		host_idx_array[i] = i;
	}

	//random_shuffle(host_idx_array, host_idx_array + SIZE);

	cl_mem dev_idx_array, dev_buf_array;

	dev_idx_array = clCreateBuffer(context, CL_MEM_READ_WRITE , SIZE * sizeof(int), NULL, &clStatus);
	dev_buf_array = clCreateBuffer(context, CL_MEM_READ_WRITE , SIZE * sizeof(int), NULL, &clStatus);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_idx_array);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_buf_array);
	clSetKernelArg(kernel, 2, sizeof(cl_int), &SIZE);

	size_t global_size = SIZE;
	size_t local_size = 64;


	struct timespec start_time, end_time;
	clock_gettime(CLOCK_REALTIME, &start_time);

	cl_event event0;
	cl_event copy_time_1;
	cl_event copy_time_2;

	clEnqueueWriteBuffer(command_queue, dev_idx_array, CL_TRUE, 0, SIZE * sizeof(int), host_idx_array, 0, NULL, &copy_time_1);
	clEnqueueWriteBuffer(command_queue, dev_buf_array, CL_TRUE, 0, SIZE * sizeof(int), host_buf_array, 0, NULL, &copy_time_2);

	clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &event0);

	clWaitForEvents(1, &event0);

	unsigned long start = 0;
	unsigned long end = 0;
	unsigned long queued = 0;
	unsigned long submited = 0;

	clGetEventProfilingInfo(event0, CL_PROFILING_COMMAND_QUEUED, sizeof(queued), &queued, NULL);
	clGetEventProfilingInfo(event0, CL_PROFILING_COMMAND_SUBMIT, sizeof(submited), &submited, NULL);
	clGetEventProfilingInfo(event0, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
	clGetEventProfilingInfo(event0, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);


	unsigned long copy_queued, copy_submited, copy_start, copy_end;
	unsigned long copy_queued1, copy_submited1, copy_start1, copy_end1;

	clGetEventProfilingInfo(copy_time_1, CL_PROFILING_COMMAND_QUEUED, sizeof(copy_end), &copy_queued, NULL);
	clGetEventProfilingInfo(copy_time_1, CL_PROFILING_COMMAND_SUBMIT, sizeof(copy_submited), &copy_submited, NULL);
	clGetEventProfilingInfo(copy_time_1, CL_PROFILING_COMMAND_START, sizeof(copy_start), &copy_start, NULL);
	clGetEventProfilingInfo(copy_time_1, CL_PROFILING_COMMAND_END, sizeof(copy_end), &copy_end, NULL);


	clGetEventProfilingInfo(copy_time_2, CL_PROFILING_COMMAND_QUEUED, sizeof(copy_end1), &copy_queued1, NULL);
	clGetEventProfilingInfo(copy_time_2, CL_PROFILING_COMMAND_SUBMIT, sizeof(copy_submited1), &copy_submited1, NULL);
	clGetEventProfilingInfo(copy_time_2, CL_PROFILING_COMMAND_START, sizeof(copy_start1), &copy_start1, NULL);
	clGetEventProfilingInfo(copy_time_2, CL_PROFILING_COMMAND_END, sizeof(copy_end1), &copy_end1, NULL);

	//printf("queued =  %lu submited = %lu start = %lu end = %lu\n", queued, submited, start, end);
	unsigned long dur1 = submited - queued;
	unsigned long dur2 = start - submited;
	unsigned long dur3 = end - start;



	clEnqueueReadBuffer(command_queue, dev_buf_array, CL_TRUE, 0, SIZE * sizeof(int), host_res_buf_array, 0, NULL, NULL);


	clFlush(command_queue);
	clFinish(command_queue);

	clock_gettime(CLOCK_REALTIME, &end_time);

	unsigned long all_time = 0;
	all_time = (end_time.tv_sec - start_time.tv_sec) * 1000000000l + (end_time.tv_nsec - start_time.tv_nsec);
	printf("%d\t%lu\t%lu\t%lu\t%lu\t%lu", SIZE, dur1, dur2, dur3, end - queued, all_time);
	printf("\t%lu\n", copy_end - copy_start + copy_end1 - copy_start1);

	//printf("Total time = %f seconds\n", (double) (tv4.tv_usec - tv3.tv_usec) / 1000000 + (double) (tv4.tv_sec - tv3.tv_sec));

	bool ansRight = true;
	for (int i = 0; i < SIZE; i++) {
		if (host_res_buf_array[i] != 1) {
			ansRight = false;
		}
	}

	if (!ansRight) {
		puts("the answer is wrong");
	}

	delete[] host_res_buf_array;
	delete[] host_idx_array;
	delete[] host_buf_array;

	clReleaseMemObject(dev_buf_array);
	clReleaseMemObject(dev_idx_array);

	clReleaseEvent(event0);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	delete[] kernel_program_string;
}


int main(int argc, char *argv[]) {

	//getchar();
	cl_uint num_platforms;
	cl_int clStatus = clGetPlatformIDs(0, NULL, &num_platforms);

	printf("platform num = %d\n", num_platforms);

	cl_platform_id *platforms = new cl_platform_id[num_platforms];

	cl_uint *device_num = new cl_uint[num_platforms];

	clStatus = clGetPlatformIDs(num_platforms, platforms, NULL);

	cl_device_id **plat_device_map = new cl_device_id*[num_platforms];

	for (size_t i = 0; i < num_platforms; i++) {
		clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_num[i]);
		printf("platform %u has %u devices\n", i, device_num[i]);
		plat_device_map[i] = new cl_device_id[device_num[i]];

		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_num[i], plat_device_map[i], NULL);

		for (size_t j = 0; j < device_num[j]; j++) {
			//printf("platform id = %d ,device %d\n", i, j);


			showDeviceInfo(plat_device_map[i][j]);

			printTitle();

			for (int p = 1; p < 4000; p *= 2) {
				atomtest(plat_device_map[i][j], 1024 * p);
			}

			puts("");
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

