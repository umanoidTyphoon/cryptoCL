/*
 * mem.h
 *
 *  Created on: 20/mag/2013
 *      Author: mpiuser
 */

#ifndef MEM_H_
#define MEM_H_

#define MAX_ALLOC 32

typedef struct allocation allocation;

allocation *init_mem_layout();

int allocate(allocation *mem_layout, void **ptr, int size);

int destroy(allocation *mem_layout, void *ptr);

int destroy_all(allocation *mem_layout);

#endif /* MEM_H_ */
