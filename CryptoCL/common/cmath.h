/*
 * cmath.h
 *
 *  Created on: 21/nov/2013
 *      Author: Daniele
 */

#ifndef CMATH_H_
#define CMATH_H_

/* User-defined pow operation with long data types, since long math functions
 * in Cygwin are *not* supported: Exponentiation by squaring.  This latter is
 * the standard method for doing modular exponentiation for huge numbers in
 * asymmetric cryptography.         */

long ipowl(long base,  int exp);

#endif /* CMATH_H_ */
