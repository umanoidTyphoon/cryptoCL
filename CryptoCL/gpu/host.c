/*
 * host.c
 *
 *  Created on: 18/nov/2013
 *      Author: humanoidTyphoon
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_PLATFORMS 32
#define MAX_DEVICES   32
#define MAX_STR_LEN   1024

#define MAX_SOURCE_SIZE     0x10000         // 64 KB
#define GLOBAL_SIZE         (16*1024*1024)  // 64 MB of data per input array
#define LOCAL_SIZE          8               // work items per work group



int crackMD5(unsigned char *hash, char *cs, int passlen) {

	// create the two input vectors
	int i;
	int *A = (int*)malloc(sizeof(int)*GLOBAL_SIZE);
	int *B = (int*)malloc(sizeof(int)*GLOBAL_SIZE);
	for (i = 0; i < GLOBAL_SIZE; i++) {
		A[i] = i;
		B[i] = GLOBAL_SIZE - i;
	}

	// load the kernel source code into the array source_str
	FILE *fp = fopen("vecsum.cl", "r");
	if (!fp) exit((printf("Failed to load kernel.\n"),1));
	char* source_str = (char*)malloc(MAX_SOURCE_SIZE);
	size_t source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose( fp );

	// get platform and device information
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_int ret = clGetPlatformIDs(1, &platform_id, NULL);
	if (ret) exit((printf("clGetPlatformIDs failed\n"),1));
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, NULL);
	if (ret) exit((printf("clGetDeviceIDs failed\n"),1));

	// create an OpenCL context
	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
	if (ret) exit((printf("clCreateContext failed\n"),1));

	// create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	if (ret) exit((printf("clCreateCommandQueue failed\n"),1));

	// create memory buffers on the device for each vector
	cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, GLOBAL_SIZE * sizeof(int), NULL, &ret);
	if (ret) exit((printf("clCreateBuffer failed\n"),1));

	cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, GLOBAL_SIZE * sizeof(int), NULL, &ret);
	if (ret) exit((printf("clCreateBuffer failed\n"),1));

	cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, GLOBAL_SIZE * sizeof(int), NULL, &ret);
	if (ret) exit((printf("clCreateBuffer failed\n"),1));

	// copy the lists A and B to their respective memory buffers
	ret  = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
			GLOBAL_SIZE * sizeof(int), A, 0, NULL, NULL);
	ret |= clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0,
			GLOBAL_SIZE * sizeof(int), B, 0, NULL, NULL);
	if (ret) exit((printf("clEnqueueWriteBuffer failed\n"),1));

	// create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1,
			(const char **)&source_str,
			(const size_t *)&source_size,
			&ret);
	if (ret) exit((printf("clCreateProgramWithSource failed\n"),1));

	// build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	if (ret) {
		size_t len;
		char buffer[4096];
		printf("clBuildProgram failed\n");
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
		exit(1);
	}

	// create an OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);
	if (ret) exit((printf("clCreateKernel failed\n"),1));

	// set the arguments of the kernel
	ret  = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
	if (ret) exit((printf("clSetKernelArg failed\n"),1));

	// execute the OpenCL kernel on the list
	size_t global_item_size = GLOBAL_SIZE; // process the entire lists
	size_t local_item_size  = LOCAL_SIZE;  // divide work items into groups
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
			&global_item_size, &local_item_size, 0, NULL, NULL);
	if (ret) exit((printf("clEnqueueNDRangeKernel failed\n"),1));

	// read the memory buffer C on the device to the local variable C
	int* C = (int*)malloc(sizeof(int)*GLOBAL_SIZE);
	ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0,
			GLOBAL_SIZE * sizeof(int), C, 0, NULL, NULL);
	if (ret) exit((printf("clEnqueueReadBuffer failed\n"),1));

	// display the result to the screen
	for (i = 0; i < GLOBAL_SIZE; i++)
		if (A[i]+B[i]!=C[i]) break;
	if (i < GLOBAL_SIZE) printf("correctness test: FAILED\n");
	else                 printf("correctness test: PASSED\n");

	// clean up
	ret  = clReleaseKernel(kernel);
	ret |= clReleaseProgram(program);
	ret |= clReleaseMemObject(a_mem_obj);
	ret |= clReleaseMemObject(b_mem_obj);
	ret |= clReleaseMemObject(c_mem_obj);
	ret |= clReleaseCommandQueue(command_queue);
	ret |= clReleaseContext(context);
	if (ret) exit((printf("resource release failed\n"),1));

	free(A);
	free(B);
	free(C);
	return 0;
}

int displayGPUinfo(){

	unsigned int i, j;  // iterator variables for loops

	cl_int err;
	cl_platform_id platforms[MAX_PLATFORMS]; // IDs of all the platforms
	cl_uint num_platforms;                   // number of platforms on this machine
	char vendor[MAX_STR_LEN];                // platforms vendor string
	char opencl_version[MAX_STR_LEN];        // opencl version string
	cl_device_id devices[MAX_DEVICES];       // number of devices for each platform
	cl_uint num_devices;                     // number of devices on this machine
	char deviceName[MAX_STR_LEN];            // devices name
	cl_uint numberOfCores;                   // number of cores of on a device
	cl_long amountOfMemory;                  // amount of memory on a device
	cl_uint clockFreq;                       // clock frequency of a device
	cl_ulong maxAllocatableMem;              // maximum allocatable memory
	cl_ulong localMem;                       // local memory for a device
	cl_bool available;                       // tells if the device is available
	size_t device_wg_size;                   // max number of work items in a work group

	// get the number of platforms
	err = clGetPlatformIDs (32, platforms, &num_platforms);

	int fs1 = 32, fs2=fs1-4, fs3=fs2-4;

	// iterate over platforms
	for(i = 0; i < num_platforms; i++) {

		printf("Platform: %i\n\n", i);
		err = clGetPlatformInfo (platforms[i], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
		if (err != CL_SUCCESS) exit((printf("Error in clGetPlatformInfo"),1));

		printf("    %-*s %s\n", fs2, "Platform Vendor:", vendor);

		err = clGetDeviceIDs (platforms[i], CL_DEVICE_TYPE_ALL, sizeof(devices), devices, &num_devices);
		if (err != CL_SUCCESS) exit((printf("Error in clGetDeviceIDs"),1));

		printf("    %-*s %u\n\n", fs2, "Number of devices:", num_devices);

		// iterate over devices
		for (j = 0; j < num_devices; j++) {

			// scan in device information
			err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_NAME"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_VENDOR"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, sizeof(opencl_version), opencl_version, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_OPENCL_C_VERSION"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numberOfCores), &numberOfCores, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_MAX_COMPUTE_UNITS"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(amountOfMemory), &amountOfMemory, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_GLOBAL_MEM_SIZE"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clockFreq), &clockFreq, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_MAX_CLOCK_FREQUENCY"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxAllocatableMem), &maxAllocatableMem, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_MAX_MEM_ALLOC_SIZE"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localMem), &localMem, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_LOCAL_MEM_SIZE"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_AVAILABLE, sizeof(available), &available, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_AVAILABLE"),1));

			err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(device_wg_size), &device_wg_size, NULL);
			if (err != CL_SUCCESS) exit((printf("Error in retrieving CL_DEVICE_MAX_WORK_GROUP_SIZE"),1));

			// print out device info
			printf("    %-*s %u\n", fs2, "Device:", j);
			printf("        %-*s %s\n", fs3, "Name:", deviceName);
			printf("        %-*s %s\n", fs3, "Vendor:", vendor);
			printf("        %-*s %s\n", fs3, "OpenCL version:", opencl_version);
			printf("        %-*s %s\n", fs3, "Available:", available ? "Yes" : "No");
			printf("        %-*s %u\n", fs3, "Compute Units:", numberOfCores);
			printf("        %-*s %u MHz\n", fs3, "Clock Frequency:", clockFreq);
			printf("        %-*s %0.00f MB\n", fs3, "Global Memory:", (double)amountOfMemory/1048576);
			printf("        %-*s %0.00f MB\n", fs3, "Max Allocatable Memory:", (double)maxAllocatableMem/1048576);
			printf("        %-*s %u KB\n", fs3, "Local Memory:", (unsigned int)localMem);
			printf("        %-*s %lu\n\n", fs3, "Max work group size:", device_wg_size);
			// We want only GPU information.
			break;
		}
	}

	return 0;

}
