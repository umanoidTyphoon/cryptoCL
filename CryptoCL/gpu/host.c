/*
 * host.c
 *
 *  Created on: 18/nov/2013
 *      Author: humanoidTyphoon
 */

#include <errno.h>
#include <math.h>
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

int crackMD5(unsigned char *hash, char *cs, int passlen) {

	clut_device dev;	// device struct
	cl_event  evt;      // performance measurement event
	cl_kernel kernel;	// execution kernel
	cl_int ret;			// error code

	double td;
	int cs_len;
	long chunk, disp;

	cs_len = strlen(cs);
	disp = DISPOSITIONS(cs_len, passlen);
	chunk = DISP_PER_CORE(disp, AVAILABLE_CORES);

	clut_open_device(&dev, PATH_TO_KERNEL);
	clut_print_device_info(&dev);


	/* ----------------------------------------- Create execution kernel ----------------------------------------- */
	kernel = clCreateKernel(dev.program, KERNEL_NAME, &ret);
	clut_check_err(ret, "Fallita la creazione del kernel");


	/* ----------------------------------- Create memory buffers on the device ----------------------------------- */
	cl_mem dchunk = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, sizeof(long), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione del chunk");

	cl_mem dhash = clCreateBuffer(dev.context, CL_MEM_READ_ONLY, HASH_SIZE * sizeof(unsigned char), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione dell'hash");

	cl_mem charset = clCreateBuffer(dev.context, CL_MEM_READ_ONLY, cs_len * sizeof(char), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione del charset");

	cl_mem charset_size = clCreateBuffer(dev.context, CL_MEM_READ_ONLY, sizeof(int), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione della taglia del charset");

	cl_mem dpasslen = clCreateBuffer(dev.context, CL_MEM_READ_ONLY, sizeof(int), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione della taglia del charset");

	cl_mem sync = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione del flag di sync");

	cl_mem dcracked = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, passlen * sizeof(char) + 1, NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione della password in chiaro");

	cl_mem computed_hash = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, HASH_SIZE * sizeof(unsigned char), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione della password in chiaro");


	/* ----------------------------------- Write memory buffers on the device ------------------------------------ */
	ret = clEnqueueWriteBuffer(dev.queue, dchunk, CL_TRUE, 0, sizeof(long), &chunk, 0, NULL, NULL);
	if(ret)
	   clut_panic(ret, "Fallita la scrittura del chunk sul buffer di memoria del device");

	ret = clEnqueueWriteBuffer(dev.queue, dhash, CL_TRUE, 0, HASH_SIZE * sizeof(unsigned char), hash, 0, NULL, NULL);
	if(ret)
	   clut_panic(ret, "Fallita la scrittura dell'hash sul buffer di memoria del device");

	ret = clEnqueueWriteBuffer(dev.queue, charset, CL_TRUE, 0, cs_len * sizeof(char), cs, 0, NULL, NULL);
	if(ret)
	   clut_panic(ret, "Fallita la scrittura del charset sul buffer di memoria del device");

	ret = clEnqueueWriteBuffer(dev.queue, charset_size, CL_TRUE, 0, sizeof(int), &cs_len, 0, NULL, NULL);
	if(ret)
	   clut_panic(ret, "Fallita la scrittura della taglia del charset sul buffer di memoria del device");

	ret = clEnqueueWriteBuffer(dev.queue, dpasslen, CL_TRUE, 0, sizeof(int), &passlen, 0, NULL, NULL);
	if(ret)
	   clut_panic(ret, "Fallita la scrittura della taglia del charset sul buffer di memoria del device");

	/* --------------------------------- Set the arguments to our compute kernel --------------------------------- */
	ret  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dchunk);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dhash);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &charset);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &charset_size);
	ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &dpasslen);
	ret |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &sync);
	ret |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &dcracked);
	ret |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &computed_hash);
	clut_check_err(ret, "Fallito il setting degli argomenti del kernel");


	/* ---------------------------------------- Execute the OpenCL kernel ---------------------------------------- */
	size_t global_dim[] = { 1 };
	size_t local_dim[]  = { 1 };
	ret = clEnqueueNDRangeKernel(dev.queue, kernel, 1, NULL, global_dim, local_dim, 0, NULL, &evt);
	if(ret)
	   clut_check_err(ret, "Fallita l'esecuzione del kernel");


	/* -------------------------- Read the device memory buffer to the local variable ---------------------------- */
	char *cracked = (char *) malloc(passlen * sizeof(char));
	ret = clEnqueueReadBuffer(dev.queue, dcracked, CL_TRUE, 0, passlen * sizeof(char), cracked, 0, NULL, NULL);
	clut_check_err(ret, "Fallimento nel leggere il risultato di output");

	printf("Password trovata: %s\n", cracked);


	/* ------------------------------------- Return kernel execution time ---------------------------------------- */
	td = clut_get_duration(evt);


	/* ----------------------------------------------- Clean up -------------------------------------------------- */
	ret  = clReleaseKernel(kernel);
	ret |= clReleaseMemObject(dchunk);
	ret |= clReleaseMemObject(dhash);
	ret |= clReleaseMemObject(charset);
	ret |= clReleaseMemObject(sync);
	ret |= clReleaseMemObject(dcracked);
	clut_check_err(ret, "Rilascio di risorse fallito");

	clut_close_device(&dev);

	return 0;
}
