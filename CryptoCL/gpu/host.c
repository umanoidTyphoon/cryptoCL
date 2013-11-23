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
#include "verbose.h"
/* Header file containing the MACRO which expands to the path
 * of the OpenCL kernel */
#include "private.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

int crackMD5(char *hash, char *cs, int passlen) {

	clut_device dev;	// device struct
	cl_event  evt;      // performance measurement event
	cl_kernel kernel;	// execution kernel
	cl_int ret;			// error code

	double td;
	int cs_len;
	long chunk, disp;
	unsigned char bin_hash[HASH_SIZE];

	cs_len = strlen(cs);
	strToBin(hash, bin_hash, 2*HASH_SIZE);

	disp = DISPOSITIONS(cs_len, passlen);
	chunk = DISP_PER_CORE(disp, AVAILABLE_CORES);

	debug("HOST", "Numero di disposizione da calcolare per stream processing unit = %lu\n", chunk);

	clut_open_device(&dev, PATH_TO_KERNEL);
	//clut_print_device_info(&dev);


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

	cl_mem dcracked = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, HASH_SIZE, NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione della password in chiaro");

	cl_mem computed_hash = clCreateBuffer(dev.context, CL_MEM_READ_WRITE, HASH_SIZE * sizeof(unsigned char), NULL, &ret);
	if (ret)
		clut_panic(ret, "Fallita l'allocazione della memoria sul device per la memorizzazione della password in chiaro");


	/* ----------------------------------- Write memory buffers on the device ------------------------------------ */
	ret = clEnqueueWriteBuffer(dev.queue, dchunk, CL_TRUE, 0, sizeof(long), &chunk, 0, NULL, NULL);
	if(ret)
	   clut_panic(ret, "Fallita la scrittura del chunk sul buffer di memoria del device");

	ret = clEnqueueWriteBuffer(dev.queue, dhash, CL_TRUE, 0, HASH_SIZE * sizeof(unsigned char), (int *)bin_hash, 0, NULL, NULL);
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
	size_t global_dim[] = { AVAILABLE_CORES };
	size_t local_dim[]  = { 1 };
	ret = clEnqueueNDRangeKernel(dev.queue, kernel, 1, NULL, global_dim, NULL, 0, NULL, &evt);
	if(ret)
	   clut_check_err(ret, "Fallita l'esecuzione del kernel");


	/* -------------------------- Read the device memory buffer to the local variable ---------------------------- */
	int   found = -1;
	int digest[HASH_SIZE/sizeof(int)];
	char *password = (char *) malloc(passlen * sizeof(char) + 1);
	memset(password, 0, passlen * sizeof(char) + 1);

	ret = clEnqueueReadBuffer(dev.queue, sync, CL_TRUE, 0, sizeof(int), &found, 0, NULL, NULL);
	if(ret)
	   clut_check_err(ret, "Fallimento nel leggere se la password e' stata trovata con successo");
	debug("HOST", "La password e' stata trovata dal kernel OpenCL? ");
	if(found){
	   ret = clEnqueueReadBuffer(dev.queue, dcracked, CL_TRUE, 0, HASH_SIZE, digest, 0, NULL, NULL);
	   if(ret)
	      clut_check_err(ret, "Fallimento nel leggere la password");
	   printf("Si. Password: %s\n", (char *)digest);
	}
	else
		printf("No.\n\n");

	/* ------------------------------------- Return kernel execution time ---------------------------------------- */
	td = clut_get_duration(evt);
	debug("HOST","Kernel duration: %f secs\n", td);

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

void from_int_to_ASCII(int *to_Convert, char* converted, int length){
	int i;
	for(i=0; i<length; i++){
		int int_to_convert = to_Convert[i];
		from_int_to_chars(int_to_convert, converted + (i*sizeof(int)));
	}
}

void from_int_to_chars(int integer, char *s){
	int i;
	for(i=0; i<sizeof(int); i++){


	}
}
