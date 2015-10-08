#pragma once

#include <CL/cl.h>
#include "sortcommon.h"

enum Kernels {
	COPY_KERNEL,
	PARALLEL_SELECTION_KERNEL,
	PARALLEL_SELECTION_BLOCKS_KERNEL,
	PARALLEL_SELECTION_LOCAL_KERNEL,
	PARALLEL_MERGE_LOCAL_KERNEL,
	PARALLEL_BITONIC_LOCAL_KERNEL,
	PARALLEL_BITONIC_A_KERNEL,
	PARALLEL_BITONIC_B2_KERNEL,
	PARALLEL_BITONIC_B4_KERNEL,
	PARALLEL_BITONIC_B8_KERNEL,
	PARALLEL_BITONIC_B16_KERNEL,
	PARALLEL_BITONIC_C2_KERNEL,
	PARALLEL_BITONIC_C4_KERNEL,
	NB_KERNELS
} ;

 int bitonic_local = PARALLEL_BITONIC_LOCAL_KERNEL;

const char * KernelNames[NB_KERNELS + 1] = {
	"Copy","ParallelSelection","ParallelSelection_Blocks","ParallelSelection_Local", "ParallelMerge_Local", "ParallelBitonic_Local",
	"ParallelBitonic_A", "ParallelBitonic_B2", "ParallelBitonic_B4", "ParallelBitonic_B8", "ParallelBitonic_B16",
	"ParallelBitonic_C2", "ParallelBitonic_C4",
	0 };



inline void setKey(data_t & a, cl_uint key) { a.s[0] = key; }
inline void setValue(data_t & a, cl_uint value) { a.s[1] = value; }
inline cl_uint getKey(const data_t & a) { return a.s[0]; }
inline cl_uint getValue(const data_t & a) { return a.s[1]; }

inline bool operator < (const data_t & a, const data_t & b) { return getKey(a) < getKey(b); }
inline bool operator == (const data_t & a, const data_t & b) { return (getKey(a) == getKey(b)) && (getValue(a) == getValue(b)); }
inline bool operator != (const data_t & a, const data_t & b) { return (getKey(a) != getKey(b)) || (getValue(a) != getValue(b)); }
