/*
 * hash.h
 *
 *  Created on: 10/mag/2013
 *      Author: mpiuser
 */

#ifndef HASH_H_
#define HASH_H_

#define A_HEX_VALUE 10	/// Valore del letterale 'A' per la rappresentazione esadecimale
#define HIGH_BYTE 4
#define HASH_SIZE 16	/// Dimensione in byte di un hash MD5
#define BASE 10			/// Base di lavoro per la conversione di una stringa in long tramite 'strtol'
#define BASE_HASH 16	/// Base di conversione della sequesnza di byte che rappresenta lo hash target della password


/**
 * Calcola lo hash MD5 per una qualsiasi stringa in input, il risultato
 * viene memorizzato a partire dal puntatore a carattere 'hash'.
 *
 * @param plain - Puntatore a carattere che rappresenta la stringa su cui calcolare lo hash
 * @param hash . Puntatore a carattere che conterrà il valore di hash calcolato
 *
 * @return Zero in caso di successo, un codice di errore altrimenti //TODO: gestione errori
 */
int hashMD5(char *plain, unsigned char *hash);

/**
 * Converte un carattere esadecimale nel corrispettivo valore binario.
 *
 * @param c - Carattere da convertire
 *
 * @return Restituisce un intero corrispondente al valore esadecimale associato
 * al carattere in input; in caso di errore restituisce un valore negativo
 */
int hexToBin(unsigned char c);

/**
 * Converte una stringa binaria che rappresenta un hash nella corrispettiva
 * stringa esadecimale e la scrive nel buffer passato in input.
 *
 * @param hash - Hash da convertire
 * @param buffer - Buffer su cui viene scritta la stringa esadecimale
 *
 * @return void
 */
void binToHex(unsigned char *hash, char *buffer);


/**
 * Funzione ausiliaria per la stampa a console del valore dello hash
 * cui corrisponde la stringa binaria passata in input
 *
 * @param hash - Puntatore a carattere rappresentante la stringa da stampare
 */
void printHash(unsigned char *hash);


/**
 * Converte una rappresentazione esadecimale in formato stringa nella corrispettiva
 * rappresentazione binaria.
 *
 * @param string - Puntatora a carattere della stringa da converire
 * @param buffer - Puntatore alla zona di memoria dove memorizzare il risultato
 * @param size - Numero di caratteri che la funzione si aspetta di dover elaborare
 *
 * @return Zero in caso di successo, un codice di errore altrimenti //TODO: gestione errori
 */
int strToBin(char *string, unsigned char *buffer, int size);


/**
 * Effettua il confronto dei due hash passati come argomento e restituisce un intero.
 * Nel caso i due codici hash siano uguali viene ritornato il valore 0, altrimenti
 * un intero positivo.
 *
 * @param hash1 - Codice hash a 16 caratteri
 * @param hash2 - Codice hash a 16 caratteri
 *
 * @return Zero in caso 'hash1' e 'hash2' siano uguali oppure -1 se 'hash1' è minore, 1 se maggiore.
 */
int hashcmp(char *hash1, char *hash2);

#endif /* HASH_H_ */
