/*
 * cmath.c
 *
 *  Created on: 21/nov/2013
 *      Author: Daniele
 */

#include <math.h>

#include "cmath.h"

long cpowl(int base, int exp){
	long res = base;
	res = (res << (base/exp)) + ((base % exp) * base);
	return res;
}
