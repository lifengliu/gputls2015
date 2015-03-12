/*
 * TraceSet.h
 *
 *  Created on: Mar 12, 2015
 *      Author: hyliu
 */

#ifndef GPUTLS001_TRACESET_H_
#define GPUTLS001_TRACESET_H_

template <int THREAD_TRACE_SIZE>
struct TraceSet {
	int size;
	int indices[THREAD_TRACE_SIZE];
};

#endif /* GPUTLS001_TRACESET_H_ */
