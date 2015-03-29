#include <cstdio>
#include <CL/opencl.h>
#include "gputlsconsts.h"
#include "utils.h"
#include <algorithm>
#include <sys/time.h>

#include "BeforeCheckingExamples.h"
#include "BeforeCheckingExample2.h"
#include "LRPDspecExamples.h"

using std::fill;

typedef struct TraceNode {
    cl_int size;
    cl_int indices[TRACE_SIZE];  // record index, not address
    //for the case containing multiple arrays, maybe a code generator is needed.
} TraceNode;


int NUM_VALUES;
int FUNC_LOOP_NUM;

float *host_A, *host_B;
int *host_P, *host_Q;
float *host_A2, *host_B2;



static void init_assign_mem(int NUM_VALUES, int FUNC_LOOP_NUM) {

	host_A = new float[NUM_VALUES];
	host_B = new float[NUM_VALUES];
	host_P = new int[NUM_VALUES];
	host_Q = new int[NUM_VALUES];
	host_A2 = new float[NUM_VALUES];
	host_B2 = new float[NUM_VALUES];

}


static void initialize() {

	for (int i = 0; i < NUM_VALUES; i++) {
        host_A[i] = i;
        host_P[i] = i;
        host_Q[i] = i;
    }

}



static float func(int i) {
	float res = 0.0f;

	for (int p = 1; p <= FUNC_LOOP_NUM; p++) {
		res += (p + i) % 5;
	}

	return res;
}



static int sequentialTest1(double *used_time) {

	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	for (int i = 0; i < NUM_VALUES; i++) {
		host_A[host_P[i]] = func(i);

		host_B[i] = host_A[host_Q[i]];
	}


	gettimeofday(&tv2, NULL);

	*used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;

	return 0;
}


static int specTest1NoDependencies(double *used_time, int dc_on, unsigned int *effectiveMemGPUCost, unsigned int *memGPUCost) {

	cl_device_id device = gputls::getOneGPUDevice(1);    // 0 is APU; 1 is R9 290X
	//printfunc::display_device(device);

	cl_int clStatus;
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &clStatus);
	cl_command_queue command_queue = clCreateCommandQueue(context, device, 0, &clStatus);

	int sourceSize;
	char *clSourceCode = gputls::loadFile("gputls001/gputls.cl", &sourceSize);

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

	cl_kernel testTlsKernel = clCreateKernel(program, "test_kernel", &clStatus);

	cl_kernel DC1kernel = clCreateKernel(program, "dependency_checking_phase_one", &clStatus);
	cl_kernel reduceKernel = clCreateKernel(program, "reduce", &clStatus);
	cl_kernel DC3kernel = clCreateKernel(program, "dependency_checking_phase_three", &clStatus);


	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	unsigned int __memoryCostGPU = 0;
	unsigned int __effectiveMemoryCostGPU = 0;

	__memoryCostGPU += NUM_VALUES * sizeof(float);
	__memoryCostGPU += NUM_VALUES * sizeof(float);
	__memoryCostGPU += NUM_VALUES * sizeof(int);
	__memoryCostGPU += NUM_VALUES * sizeof(int);

	__effectiveMemoryCostGPU = __memoryCostGPU;

	__memoryCostGPU += NUM_VALUES * sizeof(TraceNode);
	__memoryCostGPU += NUM_VALUES * sizeof(TraceNode);
	__memoryCostGPU += NUM_VALUES * sizeof(cl_int);
	__memoryCostGPU += NUM_VALUES * sizeof(cl_int);
	__memoryCostGPU += NUM_VALUES * sizeof(cl_int) * 3;

	*effectiveMemGPUCost = __effectiveMemoryCostGPU;
	*memGPUCost = __memoryCostGPU;

	cl_mem device_A = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(float), host_A, &clStatus);
	cl_mem device_B = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(float), host_B, &clStatus);
	cl_mem device_P = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(int), host_P, &clStatus);
	cl_mem device_Q = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(int), host_Q, &clStatus);

	cl_mem device_read_trace = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_VALUES * sizeof(TraceNode), NULL, &clStatus);
	cl_mem device_write_trace = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_VALUES * sizeof(TraceNode), NULL, &clStatus);


	int *emptyIntArray = new int[NUM_VALUES];
	fill(emptyIntArray, emptyIntArray + NUM_VALUES, 0);

	cl_mem device_read_to = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(cl_int), emptyIntArray, &clStatus);
	cl_mem device_write_to = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(cl_int), emptyIntArray, &clStatus);
	cl_mem device_write_count = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUM_VALUES * sizeof(cl_int), emptyIntArray, &clStatus);

	int spec = 0;
	cl_mem device_misspeculation = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &spec, &clStatus);


	clSetKernelArg(testTlsKernel, 0, sizeof(cl_mem), &device_A);
	clSetKernelArg(testTlsKernel, 1, sizeof(cl_mem), &device_B);
	clSetKernelArg(testTlsKernel, 2, sizeof(cl_mem), &device_P);
	clSetKernelArg(testTlsKernel, 3, sizeof(cl_mem), &device_Q);
	clSetKernelArg(testTlsKernel, 4, sizeof(cl_int), &FUNC_LOOP_NUM);
	clSetKernelArg(testTlsKernel, 5, sizeof(cl_mem), &device_read_trace);
	clSetKernelArg(testTlsKernel, 6, sizeof(cl_mem), &device_write_trace);

	size_t global_size = NUM_VALUES;
	size_t local_size = 64;



	clStatus = clEnqueueNDRangeKernel(command_queue, testTlsKernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	clEnqueueReadBuffer(command_queue, device_A, CL_TRUE, 0, NUM_VALUES * sizeof(float), host_A, 0, NULL, NULL);

	clFlush(command_queue);
	clFinish(command_queue);

	/*for (int i = 0; i < 5; i ++ ){
		printf("%.2f\n", host_A[i]);
	}

	puts("this finishes speculative execution, next we perform dependency checking \n\n\n");
	 */

	if (dc_on) {
		// phase 1
		clSetKernelArg(DC1kernel, 0, sizeof(cl_mem), &device_read_trace);
		clSetKernelArg(DC1kernel, 1, sizeof(cl_mem), &device_write_trace);
		clSetKernelArg(DC1kernel, 2, sizeof(cl_mem), &device_read_to);
		clSetKernelArg(DC1kernel, 3, sizeof(cl_mem), &device_write_to);
		clSetKernelArg(DC1kernel, 4, sizeof(cl_mem), &device_write_count);

		clStatus = clEnqueueNDRangeKernel(command_queue, DC1kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

		//int tmp_output[100];
		//clEnqueueReadBuffer(command_queue, device_write_count, CL_TRUE, 0, 100 * sizeof(int), tmp_output, 0, NULL, NULL);

		/*for (int i = 0; i < 100; i++) {
			printf("%d ", tmp_output[i]);
		}

		puts("");
		*/

		//reduction on device_write_to

		size_t localWorkSize = 128;
		size_t numWorkGroups = 64;
		size_t globalWorkSize = numWorkGroups * localWorkSize;

		cl_mem device_reducedWriteTo = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numWorkGroups * sizeof(int), NULL, &clStatus);

		clSetKernelArg(reduceKernel, 0, sizeof(cl_mem), &device_write_to);
		clSetKernelArg(reduceKernel, 1, sizeof(cl_int) * localWorkSize, NULL);
		clSetKernelArg(reduceKernel, 2, sizeof(cl_int), &NUM_VALUES);
		clSetKernelArg(reduceKernel, 3, sizeof(cl_mem), &device_reducedWriteTo);

		clStatus = clEnqueueNDRangeKernel(command_queue, reduceKernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);

		int *reduced_writeToResult = new int[numWorkGroups];
		clEnqueueReadBuffer(command_queue, device_reducedWriteTo, CL_TRUE, 0, sizeof(int) * numWorkGroups, reduced_writeToResult, 0, NULL, NULL);

		int writeToSum = 0;
		for (size_t i = 0; i < numWorkGroups; i++) {
			writeToSum += reduced_writeToResult[i];
		}

		//printf("writeToSum = %d\n", writeToSum);


		// --------------------------------------------------------------------------------------------------------------
		// ------------------------what the fuck ... need to be refactoried ..................
		// --------------------------------------------------------------------------------------------------------------

		cl_mem device_reducedWriteCount = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numWorkGroups * sizeof(int), NULL, &clStatus);

		clSetKernelArg(reduceKernel, 0, sizeof(cl_mem), &device_write_count);
		clSetKernelArg(reduceKernel, 1, sizeof(cl_int) * localWorkSize, NULL);
		clSetKernelArg(reduceKernel, 2, sizeof(cl_int), &NUM_VALUES);
		clSetKernelArg(reduceKernel, 3, sizeof(cl_mem), &device_reducedWriteCount);

		clStatus = clEnqueueNDRangeKernel(command_queue, reduceKernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);

		int *reducedWriteCountResult = new int[numWorkGroups];

		clEnqueueReadBuffer(command_queue, device_reducedWriteCount, CL_TRUE, 0, sizeof(int) * numWorkGroups, reducedWriteCountResult, 0, NULL, NULL);

		int writeCountSum = 0;
		for (size_t i = 0; i < numWorkGroups; i++) {
			writeCountSum += reducedWriteCountResult[i];
		}

		//printf("writeCountSum = %d\n", writeCountSum);
		//puts("");

		if (writeToSum < writeCountSum) {
			spec = 1;
		}


		if (spec == 0) { // launch phase 3 to check read[i] & write[i]
			clSetKernelArg(DC3kernel, 0, sizeof(cl_mem), &device_read_to);
			clSetKernelArg(DC3kernel, 1, sizeof(cl_mem), &device_write_to);
			clSetKernelArg(DC3kernel, 2, sizeof(cl_mem), &device_misspeculation);
			clSetKernelArg(DC3kernel, 3, sizeof(cl_mem), &NUM_VALUES);

			clEnqueueNDRangeKernel(command_queue, DC3kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

			clEnqueueReadBuffer(command_queue, device_misspeculation, CL_TRUE, 0, sizeof(int), &spec, 0, NULL, NULL);

		}

		delete[] reducedWriteCountResult;
		delete[] reduced_writeToResult;

	}

	clFlush(command_queue);
	clFinish(command_queue);

	//printf("misspec  = %d \n" , spec);


	gettimeofday(&tv2, NULL);
	//printf("GPU Total time = %f seconds\n",
//			(double) (tv2.tv_usec - tv1.tv_usec) / 1000000
	//				+ (double) (tv2.tv_sec - tv1.tv_sec));

	*used_time = (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;


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

	clReleaseKernel(testTlsKernel);
	clReleaseKernel(DC1kernel);
	clReleaseKernel(DC3kernel);
	clReleaseKernel(reduceKernel);

	delete[] emptyIntArray;

	return 0;
}


static void performCompare(int testCount = 5) {

	init_assign_mem(NUM_VALUES, FUNC_LOOP_NUM);

	initialize();

	double seq_time_sum = 0;
	for (int cnt = 0; cnt < testCount; cnt++) {
		double sequential_time;
		sequentialTest1(&sequential_time);
		seq_time_sum += sequential_time;
	}
	double seq_time_avg = seq_time_sum / testCount;

	memcpy(host_A2, host_A, NUM_VALUES * sizeof(float));
	memcpy(host_B2, host_B, NUM_VALUES * sizeof(float));

	initialize();

	double par_time_sum = 0;

	unsigned int effMemCost, memCost;
	for (int cnt = 0; cnt < testCount; cnt ++) {
		double parallel_time;
		specTest1NoDependencies(&parallel_time, 1, &effMemCost, &memCost);
		par_time_sum += parallel_time;
	}

	double par_time_avg = par_time_sum / testCount;

	double par_time_sum_dc_off = 0;

	for (int cnt = 0; cnt < testCount; cnt ++) {
		double parallel_time_dc_off;
		specTest1NoDependencies(&parallel_time_dc_off, 0, &effMemCost, &memCost);
		par_time_sum_dc_off += parallel_time_dc_off;
	}

	double par_time_avg_dc_off = par_time_sum_dc_off / testCount;

	printf("%d\t%d\t%.0f\t%.0f\t%.0f\t%.2f\t%.2f\n", NUM_VALUES, FUNC_LOOP_NUM, seq_time_avg, par_time_avg, par_time_avg_dc_off, seq_time_avg / par_time_avg, seq_time_avg / par_time_avg_dc_off);

	bool yes = true;
	for (int i = 0; i < NUM_VALUES; i++) {
		if (host_A[i] != host_A2[i]) {
			yes = false;
			printf("%d %.2f %.2f\n", i, host_A[i], host_A2[i]);
			//break;
		}

		if (host_B[i] != host_B2[i]) {
			yes = false;
			printf("%d %.2f %.2f\n", i, host_A[i], host_A2[i]);
			//break;
		}
	}

	if (!yes) {
		puts("unsuccessful");
	}

	delete[] host_A;
	delete[] host_A2;
	delete[] host_B;
	delete[] host_B2;
	delete[] host_P;
	delete[] host_Q;

}

static void compare1() {
	printf("NUM_VALUES\tFUNC_LOOP\tCPU(microsecs)\tGPU(DC on)(microsecs)\tGPU(DC off)(microsecs)\tSpeedUp(DC on)\tSpeedUp(DC off)\n");

	for (int nv = 10000; nv <= 1000000; nv *= 10) {
		for (int funcIter = 10; funcIter <= 1000; funcIter *= 10) {
			NUM_VALUES = nv;
			FUNC_LOOP_NUM = funcIter;
			performCompare();
		}
	}

}

cl_device_id device = gputls::getOneGPUDevice(1);    // 0 is APU; 1 is R9 290X


static double getTimeDouble(timeval &tv1, timeval &tv2) {
	return (double) (tv2.tv_usec - tv1.tv_usec) + (double) (tv2.tv_sec - tv1.tv_sec) * 1000000;
}


static void testLR1(int loopSize, int calcSize,
		double &sequentialTime, double &parTime,
		double &dcTime, double &exeTime,
		double &speedup
		) {

	LRPDspecExamples lr(loopSize, calcSize, loopSize * 2, device);

	struct timeval tv1, tv2, tv3, tv4;
	gettimeofday(&tv1, NULL);
	lr.parallelExecute();
	gettimeofday(&tv2, NULL);
	lr.dependencyChecking();
	gettimeofday(&tv3, NULL);
	lr.sequentialExecute();
	gettimeofday(&tv4, NULL);

	dcTime = getTimeDouble(tv2, tv3);
	sequentialTime = getTimeDouble(tv3, tv4);
	exeTime = getTimeDouble(tv1, tv2);
	parTime = getTimeDouble(tv1, tv3);
	speedup = sequentialTime / parTime;
}


static void testLRPD() {
	puts("LRPD");

	int size = 5000;
	int calc = 128;
	for (int i = 1;i <= 15;i++) {
		calc = 128 * i;
		double seqTime, parTime, dcTime, exeTime, speedup;
		testLR1(size, calc, seqTime, parTime, dcTime, exeTime, speedup);
		printf("(%d, %.2f)\n", calc, speedup);
	}

}



static void testBC1(int loopSize, int calcSize,
		double &sequentialTime, double &parTime,
		double &dcTime, double &exeTime,
		double &speedup
		) {

	BeforeCheckingExamples bce(loopSize, calcSize, loopSize * 2, device);

	struct timeval tv1, tv2, tv3, tv4;
	gettimeofday(&tv1, NULL);
	bce.parallelCheck();
	gettimeofday(&tv2, NULL);
	bce.parallelExecute();
	gettimeofday(&tv3, NULL);
	bce.sequentialExecute();
	gettimeofday(&tv4, NULL);

	dcTime = getTimeDouble(tv1, tv2);
	sequentialTime = getTimeDouble(tv3, tv4);
	exeTime = getTimeDouble(tv2, tv3);
	parTime = getTimeDouble(tv1, tv3);
	speedup = sequentialTime / parTime;
}


static void testBC() {
	puts("BC");
	int size = 5000;
	int calc = 512;
	//printf("problemSize\tsequential\tparallel\tdcpart\texecution\tspeedup\n");
	for (int i = 1;i <= 15;i++) {
		int calc = 128 * i;
		double seqTime, parTime, dcTime, exeTime, speedup;
		testBC1(size, calc, seqTime, parTime, dcTime, exeTime, speedup);
		printf("(%d, %.2f)\n", calc, speedup);
	}
}


static void barChart() {
	puts("BC");
	int size = 100000;
	int calc = 1024;
	//printf("problemSize\tsequential\tparallel\tdcpart\texecution\tspeedup\n");
	for (int i = 1;i <= 10;i++) {
		size = 4096 * i;
		double EXE ;
		double seqTime, parTime, dcTime, exeTime, speedup;
		testLR1(size, calc, seqTime, parTime, dcTime, exeTime, speedup);


		EXE = exeTime;

		printf("(%d, %.2f)\n", i, parTime);

		}
}


int main (int argc, const char *argv[]) {
		//testBC();
		//testLRPD();
	//barChart();
	//printfunc::printPlatformAndDevices();
	//printfunc::printExtensions();


	//puts("input NUM_VALUES");
	//scanf("%d", &NUM_VALUES);

	//puts("input FUNC_LOOP_NUM");
	//scanf("%d", &FUNC_LOOP_NUM);

	//compare1();   to test old example



	/*




	bce.sequentialExecute();
	 */

	/*LRPDspecExamples lr(12367, 50, 50000*2, device);
	lr.parallelExecute();
	lr.dependencyChecking();
	 */

	/*
	NUM_VALUES = 1000;
	FUNC_LOOP_NUM = 100;
	init_assign_mem(NUM_VALUES, FUNC_LOOP_NUM);

	double time1;
	unsigned int effMemCost, memCost;
	specTest1NoDependencies(&time1, 1, &effMemCost, &memCost);
	printf("effectiveMemoryCost = %u bytes  memoryCost = %u bytes utilization rate = %.2f \n", effMemCost, memCost, (effMemCost+0.0) / memCost);
    */

	BeforeCheckingExample2 bce2(100, 100, 100, device);
	bce2.evaluateConditions();
 	return 0;
}



