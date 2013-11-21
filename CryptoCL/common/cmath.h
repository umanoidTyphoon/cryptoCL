/*
 * cmath.h
 *
 *  Created on: 21/nov/2013
 *      Author: Daniele
 */

#ifndef CMATH_H_
#define CMATH_H_

/* User-defined pow operation with long data types, since long math functions
 * in Cygwin are *not* supported */

long cpowl(int base,  int exp);

#endif /* CMATH_H_ */
