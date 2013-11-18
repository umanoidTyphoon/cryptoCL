/*
 * mem.c
 *
 *  Created on: 20/mag/2013
 *      Author: mpiuser
 */

#include <stdlib.h>
#include <string.h>

#include "cerrno.h"
#include "mem.h"

struct allocation {
	void *structs[MAX_ALLOC];
	int num;
};

allocation *init_mem_layout(){
	allocation *alloc = (allocation *) malloc(sizeof(allocation));
	memset(alloc, 0, sizeof(allocation));
	return alloc;
}

int allocate(allocation *mem_layout, void **ptr, int size){
	int i;

	*ptr = malloc(size);
	if(!ptr)
		return -EALLOC;

	for(i=0; i<MAX_ALLOC; i++){
		if(!mem_layout->structs[i])
			break;
	}

	mem_layout->structs[i] = *ptr;
	mem_layout->num++;

	return -SUCCESS;
}

int destroy(allocation *mem_layout, void *ptr){
	int i;

	for(i=0; i<MAX_ALLOC; i++){
		if(mem_layout->structs[i] == ptr){
			mem_layout->structs[i] = 0;
			free(ptr);
			mem_layout->num--;
			return SUCCESS;
		}
	}

	return -EALLOC;
}

int destroy_all(allocation *mem_layout){
	int i;
	void *ptr;

	for(i=0; i<MAX_ALLOC; i++){
		ptr = mem_layout->structs[i];
		if(ptr != NULL){
			mem_layout->structs[i] = 0;
			mem_layout->num = 0;
			free(ptr);
		}
	}

	return SUCCESS;
}
