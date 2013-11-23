/*
 * shell.h
 *
 *  Created on: 15/nov/2013
 *      Author: humanoidTyphoon
 */

#ifndef SHELL_H_
#define SHELL_H_

#define cmd_ready(flag) ((flag) & 0x1h)
#define passwd_ready(flag) ((flag) & 0x2h)

#define CMD_QUIT 0
#define CMD_EXEC 1
#define CMD_ABRT 2
#define PERMS 0666
#define STR_INT_SIZE 16

/* Print the PROMPT macro, declared in "verbose.h", at each iteration of
 * the main I/O loop of the shell
 */
void print_sh_sym(char *promt_sym, int *iteration);

/**
 * Reppresenta il main loop di I/O per l'impostazione delle variabili di lavoro
 * e l'esecuzione dei comandi di ricerca delle chiavi
 */
void shell();

void sh_abort_mpi();

void quit();

void sig_halt();

int wait_child();


#endif /* SHELL_H_ */
