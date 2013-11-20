/*
 * host.h
 *
 *  Created on: 18/nov/2013
 *      Author: humanoidTyphoon
 */

#ifndef HOST_H_
#define HOST_H_

#define GLOBAL_SIZE         (16*1024*1024)  // 64 MB of data per input array
#define LOCAL_SIZE          8               // work items per work group
#define KERNEL_NAME			"crackMD5"

int crackMD5(unsigned char *hash, char *cs, int passlen);
int displayGPUinfo();

#endif /* HOST_H_ */
