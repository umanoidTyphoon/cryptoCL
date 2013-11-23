/*
 * shell.c
 *
 *  Created on: 15/nov/2013
 *      Author: humanoidTyphoon
 */

/* Compiler's directive to ignore warnings! */
#pragma GCC diagnostic ignored "-Wpointer-sign"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "cerrno.h"
#include "hash.h"
#include "host.h"
#include "shell.h"
#include "struct.h"
#include "sym.h"
#include "verbose.h"


extern char *charsets[];
extern char *help_msg;

pid_t gpu_process = 0;
user_input *ui;

/**
 * Gestore dei segnali di terminazione della shell
 */
void sig_halt(){

	quit();
	printf("Exit...\n");
	exit(0);
}

void quit(){
	free(ui);
}

void shell() {

	char num_procs[32];
	char spasslen[STR_INT_SIZE], sverbose[STR_INT_SIZE], sauditing[STR_INT_SIZE];
	int num, ret;
	unsigned char buffer[256], buffer2[256], *token;
	unsigned char hash[HASH_SIZE];
	pthread_t wait_id;

	printf("===============================\n");
	printf("           Crypto\n");
	printf("===============================\n");



	/* Azzeramento aree di memoria */
	bzero(ui, sizeof(user_input));
	strcpy(num_procs, "1");

	/* Setting del charset di default - alfabeto minuscolo */
	memcpy(ui->cs, charsets[0], strlen(charsets[0]) + 1);
	printf("\nINFO : Charset di default impostato correttamente\n\n");

	/* Arma i segnali di terminazione forzata */
	signal(SIGHUP, sig_halt);
	signal(SIGINT, sig_halt);
	signal(SIGQUIT, sig_halt);
	signal(SIGTERM, sig_halt);

	/* Avvia il main loop di shell per la configurazione */
	while (1) {
		printf(PROMPT);

		bzero(buffer, sizeof(buffer));
		fgets(buffer, sizeof(buffer), stdin);

		token = strtok(buffer, " \n");
		if (token == NULL ) {
			continue;
		}

		// -------------- HELP -----------------
		else if (!strcmp(token, "help")) {
			printf("%s\n", help_msg);
		}

		// -------------- SETS -----------------
		else if (!strcmp(token, "set")) {
			token = strtok(NULL, " \n");

			if(token == NULL){
				printf("usage: set [passwd | passlen | cs] {value}\n");
				continue;
			}

			if (!strcmp(token, "passlen")) {
				token = strtok(NULL, " \n");

				ret = (int) strtol(token, NULL, BASE);
				if (ret < 1) {
					printf("La lunghezza della password deve essere maggiore di zero!\n");
					continue;
				}

				ui->passlen = ret;
				printf("Impostata una lunghezza di password di %d\n",
						ui->passlen);
			}

			if (!strcmp(token, "passwd")) {
				token = strtok(NULL, " \n");

				if(token == NULL){
					printf("usage: set passwd {MD5-hash}\n");
					continue;
				}

				if(strchr(token, 'x') != NULL)
					token = strchr(token, 'x')+1;

				if (strlen(token) - 2*HASH_SIZE) {
					printf("Stringa non valida\n");
					continue;
				}
				strncpy(ui->hash, token, 2*HASH_SIZE+1);

				printf("Impostata l'hash della password target = ");
				printf("%s\n", ui->hash);

			}

			if (!strcmp(token, "cs")) {
				token = strtok(NULL, " \n");

				if(token == NULL){
					printf("usage: set cs [0, 6]\n");
					continue;
				}

				num = (int) strtol(token, NULL, BASE);
				if (num < 0 || num > CS_SIZE) {
					printf("Range non valido [0, 6]\n");
					continue;
				}

				memcpy(ui->cs, charsets[num], strlen(charsets[num]) + 1);
				printf("Impostato il charset = '%s'\n", ui->cs);
			}
		}

		// -------------- RUN -----------------
		else if (!strcmp(token, "run")) {
			pprintf("SHELL", "Avvio procedura decrittazione con parametri:\n");
			printf("\tcharset = '%s'\n", ui->cs);
			printf("\tpasswd: %s", ui->hash);
			printf("\n\tpasslen = %d\n", ui->passlen);
			printf("\tAuditing %s\n", ui->auditing ? "abilitato" : "disabilitato");
			printf("\tConferma? (y, n) ");

			while (!fgets(buffer, sizeof(buffer), stdin)) {
				continue;
			}

			if (buffer[0] == 'y') {
				if(ui->cs == 0 || ui->passlen <= 0 || ui->hash == 0){
					printf("Parametri non corretti!\nEsecuzione annullata\n");
					continue;
				}

				printf("\n\n");
				debug("SHELL", "Avvio GPU Computing...\n");
				ret = 0;

				sprintf(spasslen, "%d", ui->passlen);
				sprintf(sverbose, "%d", ui->verbose);
				sprintf(sauditing, "%d", ui->auditing);


				gpu_process = fork();
				if (!gpu_process){
					ret = crackMD5(ui->hash, ui->cs, ui->passlen);
					exit(ret);
				}

				debug("SHELL", "GPU process avviato con PID = %d\n", gpu_process);
				pthread_create(&wait_id, NULL, (void *)wait_child, NULL);
			}

			else {
				printf("Esecuzione annullata\n");
				continue;
			}
		}


		// -------------- HASH -----------------
		else if (!strcmp(buffer, "hash")) {
			token = strtok(NULL, " \n");
			if(token == NULL){
				printf("usage: hash [plain-text]\n");
				continue;
			}

			hashMD5(token, hash);
			binToHex(hash, buffer2);
			printf("> MD5('%s') = ", token);
			printHash(hash);
			printf("\n");

			// Autosets
			strcpy(ui->hash, buffer2);
			ui->passlen = strlen(token);
			printf("Impostato automaticamente:\n");
			printf("passwd hash = %s\n", ui->hash);
			printf("passlen = %d\n", ui->passlen);

		}

		// -------------- VERBOSE -----------------
		else if (!strcmp(buffer, "verbose")) {
			token = strtok(NULL, " \n");
			if(token == NULL){
				printf("usage: verbose {0,1}\n");
				continue;
			}

			ui->verbose = (int) strtol(token, NULL, BASE);
			if(ui->auditing) printf("Verbose mode attiva\n");
			else printf("Verbose mode disabilitata\n");
		}

		// -------------- AUDITING -----------------
		else if (!strcmp(buffer, "auditing")) {
			token = strtok(NULL, " \n");
			if(token == NULL){
				printf("usage: auditing {0,1}\n");
				continue;
			}
			ui->auditing = (int) strtol(token, NULL, BASE);
			if(ui->auditing) printf("Abilitato il processo di auditing\n");
			else printf("Disabilitato il processo di auditing\n");
		}

		// -------------- ABORT -----------------
		else if (!strcmp(buffer, "abort")){
			sig_halt();
			return;
		}

		// -------------- QUIT -----------------
		else if (!strcmp(token, "quit") ||! strcmp(token, "exit")) {
			sig_halt();
			return;
		}
	}

}

int wait_child() {
	int gpu_proc_status;

	debug("SHELL", "Attesa della terminazione del processo sulla GPU\n");
	wait(&gpu_proc_status);
	debug("SHELL", "GPU process terminato con codice = %d (%s)\n", gpu_proc_status, strerror(gpu_proc_status));

	return gpu_proc_status;
}


/*
 * -------------------------- MAIN --------------------------
 * Punto di ingresso del programma, da questo potranno essere
 * chiamati tutti i sotto moduli di Crypto
 * ----------------------------------------------------------
 */
int main(int argc, char *args[]){

	ui = malloc(sizeof(user_input));

	shell();

	quit();
	return 0;
}
