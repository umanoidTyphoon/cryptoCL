/*
 * cmath.c
 *
 *  Created on: 21/nov/2013
 *      Author: Daniele
 */

#include "cmath.h"

/* Exponentiation by squaring: this is the standard method for doing modular
 *         exponentiation for huge numbers in asymmetric cryptography.         */
long ipowl(long base, int exp){
    long result = 1;
    while (exp) {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}
