/*
 * BeforeCheckingExamples.h
 *
 *  Created on: Mar 9, 2015
 *      Author: hyliu
 */

#ifndef GPUTLS001_BEFORECHECKINGEXAMPLES_H_
#define GPUTLS001_BEFORECHECKINGEXAMPLES_H_



/*
Example 1:

for int i = 1 to 100000 do {
    a[P[i]] = b[Q[i]] + c[Q[i] * 2];
    b[T[i]] = f(i);
    d[i] = g(i);
}


In this example, a is a shared array
b is shared array
c P Q T d is not shared array

 */




class BeforeCheckingExamples {
public:
	BeforeCheckingExamples();
	virtual ~BeforeCheckingExamples();
};

#endif /* GPUTLS001_BEFORECHECKINGEXAMPLES_H_ */
