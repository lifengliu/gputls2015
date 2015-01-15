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



	cl_kernel dependencyCheckingKernel = clCreateKernel(program, "dependency_checking", &clStatus);
	cl_kernel testTlsKernel = clCreateKernel(program, "test_kernel", &clStatus);

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

	clSetKernelArg(testTlsKernel, 0, NUM_VALUES * sizeof(float), &device_A);
	clSetKernelArg(testTlsKernel, 1, NUM_VALUES * sizeof(float), &device_B);
	clSetKernelArg(testTlsKernel, 2, NUM_VALUES * sizeof(cl_int), &device_P);
	clSetKernelArg(testTlsKernel, 3, NUM_VALUES * sizeof(cl_int), &device_Q);
	clSetKernelArg(testTlsKernel, 4, NUM_VALUES * sizeof(TraceNode), &device_read_trace);
	clSetKernelArg(testTlsKernel, 5, NUM_VALUES * sizeof(TraceNode), &device_write_trace);


	clSetKernelArg(dependencyCheckingKernel, 0, NUM_VALUES * sizeof(TraceNode), &device_read_trace);
	clSetKernelArg(dependencyCheckingKernel, 1, NUM_VALUES * sizeof(TraceNode), &device_write_trace);
	clSetKernelArg(dependencyCheckingKernel, 2, NUM_VALUES * sizeof(cl_int), &device_read_to);
	clSetKernelArg(dependencyCheckingKernel, 3, NUM_VALUES * sizeof(cl_int), &device_write_to);
	clSetKernelArg(dependencyCheckingKernel, 4, NUM_VALUES * sizeof(cl_int), &device_write_count);
	clSetKernelArg(dependencyCheckingKernel, 5, sizeof(cl_int), &device_misspeculation);

	size_t global_size = NUM_VALUES;
	size_t local_size = 64;

	clStatus = clEnqueueNDRangeKernel(command_queue, testTlsKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	clEnqueueReadBuffer(command_queue, device_A, CL_TRUE, 0, NUM_VALUES * sizeof(float), host_A, 0, NULL, NULL);

	clStatus = clEnqueueNDRangeKernel(command_queue, dependencyCheckingKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	int spec = -1;
	clEnqueueReadBuffer(command_queue, device_misspeculation, CL_TRUE, 0, sizeof(cl_int), &spec, 0, NULL, NULL);


	clFlush(command_queue);
	clFinish(command_queue);

	for (int i = 0; i < 5; i ++ ){
		printf("%.2f\n", host_A[i]);
	}

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

	//clReleaseKernel()



/*    char device_name[128];
    cl_context context = gcl_get_context();
    
    size_t length;
    cl_device_id devices[8];
    clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(devices), devices, &length);
    
    fprintf(stdout, "The following devices are available for use: \n");
    
    int num_devices = (int) (length / sizeof(cl_device_id));
    for (int i = 0; i < num_devices; i++) {
        display_device(devices[i]);
    }
    
    dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
    
    if (queue == NULL) {
        printf("cannot create a GPU dispatch queue, so create on CPU\n");
        queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
    }
    
    
    cl_device_id gpu = gcl_get_device_id_with_dispatch_queue(queue);
    clGetDeviceInfo(gpu, CL_DEVICE_NAME, 128, device_name, NULL);
    fprintf(stdout, "Created a dispatch queue using the %s\n", device_name);
    
    
    
    
    //array A B P Q 1024
    
    float *host_A = (float *) malloc(sizeof(cl_float) * NUM_VALUES);
    float *host_B = (float *) malloc(sizeof(cl_float) * NUM_VALUES);
    
    int *host_P = (int *) malloc(sizeof(cl_int) * NUM_VALUES);
    int *host_Q = (int *) malloc(sizeof(cl_int) * NUM_VALUES);
    
    for (int i = 0; i < NUM_VALUES; i++) {
        host_A[i] = i;
        host_P[i] = i;
        host_Q[i] = i;
    }
    
    
    void *device_A = gcl_malloc(sizeof(cl_float) * NUM_VALUES, host_A, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR);
    
    void *device_P = gcl_malloc(sizeof(cl_int) * NUM_VALUES, host_P, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR);
    
    void *device_Q = gcl_malloc(sizeof(cl_int) * NUM_VALUES, host_Q, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR);
    
    void *device_B = gcl_malloc(sizeof(cl_float) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    
    void *device_read_set = gcl_malloc(sizeof(struct TraceNode) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    void *device_write_set = gcl_malloc(sizeof(struct TraceNode) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    
    void *device_read_to = gcl_malloc(sizeof(cl_int) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    void *device_write_to = gcl_malloc(sizeof(cl_int) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    void *device_write_count = gcl_malloc(sizeof(cl_int) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    
    void *device_misspeculation = gcl_malloc(sizeof(cl_int), NULL, CL_MEM_READ_WRITE);
    
    
    dispatch_sync(queue, ^{
        
//        cl_ulong local_memsize, private_memsize;
//        
//        gcl_get_kernel_block_workgroup_info(square_kernel, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(local_memsize), &local_memsize, NULL);
//        
//        fprintf(stdout, "Local memory size: %lld\n", local_memsize);
//        
//        gcl_get_kernel_block_workgroup_info(square_kernel, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(private_memsize), &private_memsize, NULL);
//        
//        fprintf(stdout, "Private memory size : %lld\n", private_memsize);
//        
        size_t workgroup_size;
        
        gcl_get_kernel_block_workgroup_info(test_kernel_kernel,
                                            CL_KERNEL_WORK_GROUP_SIZE,
                                            sizeof(workgroup_size), &workgroup_size, NULL);
        
        fprintf(stdout, "workgroupsize = %lu\n", workgroup_size);
        
//        size_t kernel_prefered_work_group_size_multiple;
//        
//        gcl_get_kernel_block_workgroup_info(square_kernel, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(kernel_prefered_work_group_size_multiple), &kernel_prefered_work_group_size_multiple, NULL);
//        
//        fprintf(stdout, "kernel_prefered_work_group_size_multiple = %lu\n", kernel_prefered_work_group_size_multiple);
//
        
        cl_ndrange range = {
            1,
            {0, 0, 0},
            {NUM_VALUES, 0, 0},    // The global range—this is how many items
            {workgroup_size, 0, 0}          // The local size of each workgroup.  This
        };
        
        //square_kernel(&range,(cl_float*)device_in, (cl_float*)device_out, (struct TraceNode *) device_read_set, (struct TraceNode *) device_write_set);
    
        test_kernel_kernel(&range, (cl_float *) device_A, (cl_float *) device_B, (cl_int *) device_P, (cl_int *) device_Q, (TraceNode *) device_read_set, (TraceNode *) device_write_set);
        
        gcl_memcpy(host_B, device_B, sizeof(cl_float) * NUM_VALUES);
        
    });
    
    
    for (int i = 0;i < 10; i++) {
        printf("B [%d] = %.2f\n", i, host_B[i]);
    }
    
    
    dispatch_sync(queue, ^{
        
        size_t workgroup_size;
        
        gcl_get_kernel_block_workgroup_info(dependency_checking_kernel,
                                            CL_KERNEL_WORK_GROUP_SIZE,
                                            sizeof(workgroup_size), &workgroup_size, NULL);
        
        fprintf(stdout, "workgroupsize = %lu\n", workgroup_size);
        
        cl_ndrange range = {
            1,
            {0, 0, 0},
            {NUM_VALUES, 0, 0},    // The global range—this is how many items
            {workgroup_size, 0, 0}          // The local size of each workgroup.  This
        };
        
        dependency_checking_kernel(&range, (TraceNode *) device_read_set, (TraceNode *) device_write_set, (cl_int *) device_read_to, (cl_int *) device_write_to, (cl_int *) device_write_count, (cl_int *) device_misspeculation);
        
        int mis_spec = 0;
        
        gcl_memcpy(&mis_spec, device_misspeculation, sizeof(cl_int));
        
        printf("%d\n", mis_spec);
        
    });
    
    
    //fprintf(stdout, "%d\n", sum_out[0]);
    //fprintf(stdout, "%d\n", sum_actual);
    
    //if ( validate(host_in, host_out)) {
    //   fprintf(stdout, "All values were properly squared.\n");
    //}
    
    gcl_free(device_A);
    gcl_free(device_B);
    gcl_free(device_P);
    gcl_free(device_Q);
    
    gcl_free(device_read_set);
    gcl_free(device_write_set);
    gcl_free(device_write_to);
    gcl_free(device_read_to);
    gcl_free(device_misspeculation);
    
    dispatch_release(queue);
    */
	//printf("hello world\n");


	/*
	 *  In this way, we can create all kernels from one program
	cl_uint kernel_num;
	clStatus = clCreateKernelsInProgram(program, 0, NULL, &kernel_num);

	cl_kernel *kernels = new cl_kernel[kernel_num];
	clStatus = clCreateKernelsInProgram(program, kernel_num, kernels, NULL);

	printf("kernel num = %lu\n", kernel_num);

	for (size_t i = 0; i < kernel_num; i++) {
		char kernel_name[50];

		clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, sizeof(kernel_name), kernel_name, NULL);
		printf("kernel %d %s\n", i, kernel_name);

	}*/

 	return 0;
}



