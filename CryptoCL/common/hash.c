/*
 * hash.c
 *
 *  Created on: 10/mag/2013
 *      Author: mpiuser
 */

#include <mhash.h>
#include <string.h>
#include "hash.h"

const int binaries[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

/**
 * The function take in input a parameter already initialized!
 */
int hashMD5(char *plain, unsigned char *hash) {
	MHASH td;
	int size, i;

	td = mhash_init(MHASH_MD5);
	if (td == MHASH_FAILED) {
		return -1;		//TODO: Aggiungere codice di errore
	}
	size = strlen(plain);
	for (i = 0; i < size; i++) {
		mhash(td, plain + i, 1);
	}
	mhash_deinit(td, hash);

	return 0;
}

int hexToBin(unsigned char c) {
	int low = tolower(c);

	if (low >= '0' && low <= '9')
		return binaries[c - '0'];
	if (low >= 'a' && low <= 'f')
		return binaries[A_HEX_VALUE + c - 'a'];

	return -1;
}

void printHash(unsigned char *hash) {
	int i;

	printf("0x");

	for (i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
		printf("%.2x", hash[i]);
	}

	printf("\n");
}

void binToHex(unsigned char *hash, char *buffer) {

	int i;

	strcpy(buffer, "0x");
	for (i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
		sprintf(buffer+2+2*i, "%.2x", hash[i]);
	}
}

int strToBin(char *token, unsigned char *dest, int size) {
	int i, j, lb, hb, len;
	unsigned char toCopy;
	char *buff;

	buff = strtok(token, "xX");
	if(strlen(buff) == 1)
		token = strtok(NULL, "\n");

	len = strlen(token);
	if(len != size)
		return -1; //TODO: Codice errore specifico

	for (i = 0, j = 0; i < size; i += 2) {

		hb = hexToBin(token[i]);
		lb = hexToBin(token[i + 1]);
		toCopy = (hb << HIGH_BYTE) + lb;

		memcpy(dest + j++, &toCopy, sizeof(char));
	}

	return 0;
}

int hashcmp(char *hash1, char *hash2){
	return memcmp(hash1, hash2, HASH_SIZE);
}
