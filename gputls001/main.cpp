//
//  main.cpp
//  gputls001
//
//  Created by LIU Hongyuan on 1/3/15.
//  Copyright (c) 2015 bigthree. All rights reserved.
//

#include <cstdio>

#include <OpenCL/opencl.h>

#include "gputls.cl.h"

#define NUM_VALUES 1024

static int validate(cl_float* input, cl_float* output) {
    int i;
    for (i = 0; i < NUM_VALUES; i++) {
        if ( output[i] != (input[i] * input[i]) ) {
            fprintf(stdout,
                    "Error: Element %d did not match expected output.\n", i);
            fprintf(stdout,
                    "       Saw %1.4f, expected %1.4f\n", output[i],
                    input[i] * input[i]);
            fflush(stdout);
            return 0;
        }
    }
    return 1;
}

static void display_device(cl_device_id device) {
    char name_buf[128];
    char vendor_buf[128];
    
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(char) * 128, name_buf, NULL);
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(char) * 128, vendor_buf, NULL);
    //clGetKernelWorkGroupInfo(
    fprintf(stdout, "Using OpenCL device: %s %s\n", vendor_buf, name_buf);
    
}

int main (int argc, const char * argv[]) {
    int i;
    char device_name[128];
    cl_context context = gcl_get_context();
    
    size_t length;
    cl_device_id devices[8];
    clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(devices), devices, &length);
    
    fprintf(stdout, "The following devices are available for use: \n");
    
    int num_devices = (int) (length / sizeof(cl_device_id));
    for (i = 0; i < num_devices; i++) {
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
    
    float* host_in = (float*) malloc(sizeof(cl_float) * NUM_VALUES);
    
    for (i = 0; i < NUM_VALUES; i++) {
        host_in[i] = (cl_float) i;
    }
    
    float* host_out = (float*) malloc(sizeof(cl_float) * NUM_VALUES);
    
    void* device_in  = gcl_malloc(sizeof(cl_float) * NUM_VALUES, host_in, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
    
    void* device_out = gcl_malloc(sizeof(cl_float) * NUM_VALUES, NULL, CL_MEM_WRITE_ONLY);
    
    void *device_read_set = gcl_malloc(sizeof(struct TraceNode) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    void *device_write_set = gcl_malloc(sizeof(struct TraceNode) * NUM_VALUES, NULL, CL_MEM_READ_WRITE);
    
    dispatch_sync(queue, ^{
        
        cl_ulong local_memsize, private_memsize;
        
        gcl_get_kernel_block_workgroup_info(square_kernel, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(local_memsize), &local_memsize, NULL);
        
        fprintf(stdout, "Local memory size: %lld\n", local_memsize);
        
        gcl_get_kernel_block_workgroup_info(square_kernel, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(private_memsize), &private_memsize, NULL);
        
        fprintf(stdout, "Private memory size : %lld\n", private_memsize);
        
        size_t workgroup_size;
        
        gcl_get_kernel_block_workgroup_info(square_kernel,
                                            CL_KERNEL_WORK_GROUP_SIZE,
                                            sizeof(workgroup_size), &workgroup_size, NULL);
        
        fprintf(stdout, "workgroupsize = %lu\n", workgroup_size);
        
        size_t kernel_prefered_work_group_size_multiple;
        
        //CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
        
        gcl_get_kernel_block_workgroup_info(square_kernel, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(kernel_prefered_work_group_size_multiple), &kernel_prefered_work_group_size_multiple, NULL);
        
        fprintf(stdout, "kernel_prefered_work_group_size_multiple = %lu\n", kernel_prefered_work_group_size_multiple);
        
        cl_ndrange range = {
            1,
            {0, 0, 0},
            {NUM_VALUES, 0, 0},    // The global range—this is how many items
            {kernel_prefered_work_group_size_multiple, 0, 0}          // The local size of each workgroup.  This
        };
        
        square_kernel(&range,(cl_float*)device_in, (cl_float*)device_out, (struct TraceNode *) device_read_set, (struct TraceNode *) device_write_set);
        
        gcl_memcpy(host_out, device_out, sizeof(cl_float) * NUM_VALUES);
        
    });
    
    
    if ( validate(host_in, host_out)) {
        fprintf(stdout, "All values were properly squared.\n");
    }
    
    gcl_free(device_in);
    gcl_free(device_out);
    gcl_free(device_read_set);
    gcl_free(device_write_set);
    
    free(host_in);
    free(host_out);
    
    dispatch_release(queue);
}
