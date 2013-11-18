/*
 * struct.h
 *
 * Contiene le definizioni delle strutture dati presenti nel file
 * 'struct.c' ove riesiedono le funzioni di manipolazione.
 *
 *  Created on: 10/giu/2013
 *      Author: mpiuser
 */

#ifndef STRUCT_H_
#define STRUCT_H_

#include "hash.h"
#include "sym.h"
//#include <pthread.h>
//#include <semaphore.h>

#define MAX_THREADS 8

#define CHARSET_SIZE 64
#define STR_PASSLEN 3

#define UI_FIELDS 3
#define PADDING 2

/**
 * Struttura di rappresentazione di una stringa.
 */
typedef struct string_t string_t;

/**
 * Oggetto contenente i settaggi di partenza per il
 * thread che si occuperà della ricerca della password.
 */
typedef struct comb_settings comb_settings;

/**
 * Pacchetto dei parametri di avvio per il thread di ricerca
 * della password, contenente a sua volta i settaggi ('comb_settings')
 * e le indicazioni del charset sotto forma di stringa.
 * Nella stessa è presente un campo ove verrà caricata la stringa ('string_t')
 * rappresentante la password decodificata, se trovata.
 */
typedef struct comb_parms comb_parms;

struct string_t {
	char *str;
	int size;
};

struct comb_settings {
	int *starting_point;
	long chunk;
};

struct comb_parms {
	string_t *cs, *passwd;
	comb_settings *init;
};

/**
 * Pacchetto di informazioni passate dall'utente, tramite shell,
 * al programma principale.
 */

typedef struct user_input {
	char cs[CHARSET_SIZE + 1];
	unsigned char hash[2*HASH_SIZE+1];
	int passlen;
	int verbose, auditing;
	int attack;
} user_input;

typedef struct th_parms {
	char last_try[MAX_PASSWD_LEN];
	char plain[MAX_PASSWD_LEN];
	long count;
	int wterm;
	//sem_t mutex;
	//pthread_mutex_t lock;
	//pthread_cond_t waiting;
} th_parms;

#endif /* STRUCT_H_ */
