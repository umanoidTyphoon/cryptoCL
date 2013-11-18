/*
 * verbose.h
 *
 * Definisce le funzioni di debugging
 *
 *  Created on: 16/mag/2013
 *      Author: mpiuser
 */

#ifndef VERBOSE_H_
#define VERBOSE_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DEBUG 1
#define VERBOSE 1
#define HEAD_PMT "[%d] - %s:: "

#ifdef DEBUG

#define debug(head, ...) do {		\
		printf(HEAD_PMT, getpid(), (head));	\
		printf(__VA_ARGS__);		\
} while(0)

#define panic(head, err, ...) do {		\
	debug(head, __VA_ARGS__);				\
	return err;						\
} while(0)

#else

#define debug(...) do {} while(0)

#endif

#define rdebug(head, fname, retvalue) do {												\
	debug(head, "%s thread terminato - %d (%s)\n", fname, retvalue, strerror(retvalue));	\
	} while(0)

#define verbose(head, ...) do { 	\
	if(VERBOSE){ 					\
		printf(HEAD_PMT, getpid(), (head)); 	\
		printf(__VA_ARGS__); 		\
	}								\
	} while(0)


#define pprintf(head, ...) do {		\
		printf(HEAD_PMT, getpid(), (head));	\
		printf(__VA_ARGS__);		\
	} while(0);

#endif /* VERBOSE_H_ */
