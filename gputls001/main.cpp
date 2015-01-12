#include <cstdio>
#include <CL/opencl.h>

//#include "gputls.cl.h"
#include "gputlsconsts.h"

#include "utils.h"


int main (int argc, const char * argv[]) {
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

	printfunc::printPlatformAndDevices();
	return 0;
}



