/*
 * BeforeCheckingExamples.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: hyliu
 */

#include "BeforeCheckingExamples.h"
#include <cmath>

/*
Example 1:

for int i = 1 to 100000 do {
    a[P[i]] = b[Q[i]] + c[Q[i]];
    b[T[i]] = f(i);
    d[i] = g(i);

    some_calculation();

}


In this example, a is a shared array
b is shared array
c P Q T d is not shared array

 */


BeforeCheckingExamples::BeforeCheckingExamples(int LOOP_SIZE, int CALC_SIZE) {
	this->LOOP_SIZE = LOOP_SIZE;
	this->CALC_SIZE = CALC_SIZE;
	assign_host_memory();
}


BeforeCheckingExamples::~BeforeCheckingExamples() {
	delete[] host_a;
	delete[] host_b;
	delete[] host_c;
	delete[] host_d;

	delete[] host_P;
	delete[] host_Q;
	delete[] host_T;
}


void BeforeCheckingExamples::example1() {

}


float BeforeCheckingExamples::someCalculation() {
	float res = 0.5;
	for (int i = 0; i < CALC_SIZE; i++) {
		res = sin(res);
	}
	return res;
}


void BeforeCheckingExamples::sequentialExecute() {
	for (int i = 1; i < LOOP_SIZE; i++) {
		host_a[host_P[i]] = host_b[host_Q[i]] + host_c[host_Q[i]];
	    host_b[host_T[i]] = 500;
	    host_d[i] = someCalculation();
	}
}

void BeforeCheckingExamples::initArrayValues() {
	// this init values contain no data race

	for (int i = 0; i < LOOP_SIZE; i++) {
		host_a[i] = 0;
		host_c[i] = i % 20;
		host_d[i] = 0;
		host_b[i] = 0;

		host_P[i] = i;
		host_Q[i] = i / 2;
		host_T[i] = LOOP_SIZE - i;

	}

}

void BeforeCheckingExamples::assign_host_memory() {

	host_a = new float[LOOP_SIZE];
	host_b = new float[LOOP_SIZE];
    host_c = new float[LOOP_SIZE];
    host_d = new float[LOOP_SIZE];

    host_P = new int[LOOP_SIZE];
    host_Q = new int[LOOP_SIZE];
    host_T = new int[LOOP_SIZE];

}












