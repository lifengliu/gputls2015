#pragma once

#include <CL/cl.h>
#include <string>


class OpenCLRuntimeEnv
{

public:
	OpenCLRuntimeEnv();
	~OpenCLRuntimeEnv();

	void set_command_queue(cl_command_queue queue);
	void set_context(cl_context context);
	void set_device(cl_device_id dev_id);

	cl_command_queue get_command_queue() const;
	cl_device_id get_device_id() const;
	cl_context get_context() const;
	cl_event get_event() const;

private:
	cl_command_queue current_command_queue;
	cl_device_id device;
	cl_context context;
	cl_event ev;

};