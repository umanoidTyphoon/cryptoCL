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

#include "clut.h"
#include "hash.h"
#include "host.h"
/* Header file containing the MACRO which expands to the path
 * of the OpenCL kernel */
#include "private.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define GLOBAL_SIZE         (16*1024*1024)  // 64 MB of data per input array
#define LOCAL_SIZE          8               // work items per work group



int crackMD5(unsigned char *hash, char *cs, int passlen) {

	clut_device dev;	// device struct
	cl_event  evt;      // performance measurement event
	cl_kernel kernel;	// execution kernel
	cl_int ret;			// error code

	int cs_len = strlen(cs);
	double td;

	clut_open_device(&dev, PATH_TO_KERNEL);
	clut_print_device_info(&dev);

	/* ----------------------------------------- Create execution kernel ----------------------------------------- */
	kernel = clCreateKernel(dev.program, KERNEL_NAME, &ret);
	clut_check_err(ret, "Fallita la creazione del kernel");


	/* ----------------------------------- Create memory buffers on the device ----------------------------------- */
	cl_mem plain = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, passlen * sizeof(int), NULL, &ret);
	if (ret)
		clut_panic("Fallita l'allocazione della memoria sul device per la memorizzazione della password da testare");

	cl_mem dhash = clCreateBuffer(dev.context, CL_MEM_READ_ONLY, HASH_SIZE * sizeof(unsigned char), NULL, &ret);
	if (ret)
		clut_panic("Fallita l'allocazione della memoria sul device per la memorizzazione dell'hash");

	cl_mem charset = clCreateBuffer(dev.context, CL_MEM_READ_ONLY, cs_len * sizeof(char), NULL, &ret);
	if (ret)
		clut_panic("Fallita l'allocazione della memoria sul device per la memorizzazione del charset");

	cl_mem sync = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, sizeof(char), NULL, &ret);
	if (ret)
		clut_panic("Fallita l'allocazione della memoria sul device per la memorizzazione del flag di sync");

	cl_mem dcracked = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, passlen * sizeof(char), NULL, &ret);
	if (ret)
		clut_panic("Fallita l'allocazione della memoria sul device per la memorizzazione della password in chiaro");


	/* ----------------------------------- Write memory buffers on the device ------------------------------------ */
	ret = clEnqueueWriteBuffer(dev.queue, dhash, CL_TRUE, 0, HASH_SIZE * sizeof(unsigned char), hash, 0, NULL, NULL);
	if(ret)
	   clut_panic("Fallita la scrittura dell' hash sul buffer di memoria del device");

	ret = clEnqueueWriteBuffer(dev.queue, charset, CL_TRUE, 0, cs_len * sizeof(char), cs, 0, NULL, NULL);
	if(ret)
		clut_panic("Fallita la scrittura dell' hash sul buffer di memoria del device");


	/* --------------------------------- Set the arguments to our compute kernel --------------------------------- */
	ret  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &plain);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dhash);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &charset);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &sync);
	ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &dcracked);
	clut_check_err(ret, "Fallito il setting degli argomenti del kernel");


	/* ---------------------------------------- Execute the OpenCL kernel ---------------------------------------- */
	size_t global_item_size = GLOBAL_SIZE; // process the entire lists
	size_t local_item_size  = LOCAL_SIZE;  // divide work items into groups
	ret = clEnqueueNDRangeKernel(dev.queue, kernel, 1, NULL,&global_item_size, &local_item_size, 0, NULL, &evt);
	clut_check_err(ret, "Fallita l'esecuzione del kernel");


	/* -------------------------- Read the device memory buffer to the local variable ---------------------------- */
	char *cracked = (char *) malloc(passlen * sizeof(char));
	ret = clEnqueueReadBuffer(dev.queue, dcracked, CL_TRUE, 0, passlen * sizeof(char), cracked, 0, NULL, NULL);
	clut_check_err(ret, "Fallimento nel leggere il risultato di output");

	printf("Password trovata: %s", cracked);


	/* ------------------------------------- Return kernel execution time ---------------------------------------- */
	td = clut_get_duration(evt);


	/* ----------------------------------------------- Clean up -------------------------------------------------- */
	ret  = clReleaseKernel(kernel);
	ret |= clReleaseMemObject(plain);
	ret |= clReleaseMemObject(dhash);
	ret |= clReleaseMemObject(charset);
	ret |= clReleaseMemObject(sync);
	ret |= clReleaseMemObject(dcracked);
	clut_check_err(ret, "Rilascio di risorse fallito");

	return 0;
}
