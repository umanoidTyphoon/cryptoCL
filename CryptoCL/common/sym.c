/*
 * sym.c
 *
 *  Created on: 11/giu/2013
 *      Author: mpiuser
 */


char *charsets[] = {"abcdefghijklmnopqrstuvwxyz",
					"ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890",
					"abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890abcdefghijklmnopqrstuvwxyz",
					"1234567890ABCDEFEGHIJKLMNOPQRSTUVWXYZ",
					"1234567890abcdefghijklmnopqrstuvwxyzABCDEFEGHIJKLMNOPQRSTUVWXYZ"};

char *help_msg = "===== Help =====\n" \
		"# [quit|exit] - per uscire\n" \
		"# hash {string} - restituisce l'hash MD5 della stringa in input\n" \
		"# set [passlen | passwd | cs | proc] {value} - Imposta il valore di lavoro per la variabile indicata\n" \
		"  cs {value} - imposta il charset di lavoro per la generazione delle password.\n" \
		"    num=0 -> sole minuscole\n" \
		"    num=1 -> sole maiuscole\n" \
		"    num=2 -> soli numeri\n" \
		"    num=3 -> maiuscole e minuscole\n" \
		"    num=4 -> numeri e minuscole\n" \
		"    num=5 -> numeri e maiuscole\n" \
		"    num=6 -> completo\n" \
		" passswd {string} - Rappresenta l'hash della password codificata (md5) da decrittare\n" \
		" passlen {maxsize} - Numero massimo di caratteri della password da trovare\n" \
		" proc {value} - Imposta un numero (maggiore di zero) di processi MPI da lanciare" \
		"# dictionary {0,1} - Imposta il tipo di attacco a forza bruta o dizionario rispettivamente\n" \
		"# auditing {0,1} - Stampa informazioni addizionali circa l'esecuzione corrente dell'attacco\n" \
		"# run - per avviare la generazione delle password\n" \
		"===============\n";
