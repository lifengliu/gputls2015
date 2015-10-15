#pragma once
#include <CL/cl.h>

typedef cl_uint2 data_t;

extern inline void setKey(data_t & a, cl_uint key);
extern void setValue(data_t & a, cl_uint value);

extern inline cl_uint getKey(const data_t & a);
extern cl_uint getValue(const data_t & a);

//extern inline data_t makeData(cl_uint key, cl_uint value);

extern inline bool operator < (const data_t & a, const data_t & b);
extern inline bool operator == (const data_t & a, const data_t & b);
extern inline bool operator != (const data_t & a, const data_t & b);
