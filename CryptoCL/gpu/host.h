/*
 * host.h
 *
 *  Created on: 18/nov/2013
 *      Author: humanoidTyphoon
 */

#ifndef HOST_H_
#define HOST_H_

#define COMPUTING_UNITS 2	// Number of computing units on an ATI MOBILITY RADEON HD 4500
#define DISPOSITIONS(cs_len, pass_len) (pow((cs_len),(pass_len)));
#define DISP_PER_CORE(disp, num_procs) (((disp)/(num_procs))+1);
#define KERNEL_NAME	"GPU_crackMD5"
/* Number of stream processing units contained in a computing unit of an ATI MOBILITY RADEON
 * HD 4500. A stream processing unit is totally equivalent to a CUDA CORE */
#define STREAM_PROCESSING_UNITS 40
#define STARTING_CHAR(init, cs_size, pos) (((long)((init)/(powl(cs_size, pos))))%(cs_size));
#define AVAILABLE_CORES ((COMPUTING_UNITS)*(STREAM_PROCESSING_UNITS)) // Number of total available cores on an ATI graphic card
#define AVAILABLE_THREADS 1024


int crackMD5(char *hash, char *cs, int passlen);
/* Convert a string stored in an integer array to the corresponding ASCII string */
void from_int_to_ASCII(int *to_Convert, char *converted, int length);
/* Convert a single integer to four characters */
void from_int_to_chars(int to_Convert, char *converted);

#endif /* HOST_H_ */
