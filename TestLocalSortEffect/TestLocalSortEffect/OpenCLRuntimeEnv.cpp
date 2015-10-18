#include "OpenCLRuntimeEnv.h"
#include <cstdio>



OpenCLRuntimeEnv::OpenCLRuntimeEnv()
{
}


OpenCLRuntimeEnv::~OpenCLRuntimeEnv()
{
}

void OpenCLRuntimeEnv::set_command_queue(cl_command_queue queue)
{
	current_command_queue = queue;
}

void OpenCLRuntimeEnv::set_context(cl_context context)
{
	this->context = context;
}

void OpenCLRuntimeEnv::set_device(cl_device_id dev_id)
{
	device = dev_id;
}

cl_command_queue OpenCLRuntimeEnv::get_command_queue() const
{
	return current_command_queue;
}

cl_device_id OpenCLRuntimeEnv::get_device_id() const
{
	return device;
}

cl_context OpenCLRuntimeEnv::get_context() const
{
	return context;
}

