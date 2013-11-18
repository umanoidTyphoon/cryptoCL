/*
 * host.h
 *
 *  Created on: 18/nov/2013
 *      Author: humanoidTyphoon
 */

#ifndef HOST_H_
#define HOST_H_

#define MAX_DEVICES   32
#define MAX_PLATFORMS 32
#define MAX_STR_LEN   1024

int crackMD5(unsigned char *hash, char *cs, int passlen);
int displayGPUinfo();

#endif /* HOST_H_ */
