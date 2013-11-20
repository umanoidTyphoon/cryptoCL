/* DEFs in order to shut down C syntax errors */

#define __kernel
#define __global

__kernel void vector_add(__global int *passwd, __global const unsigned char hash, __global const char *cs,
						 __global char sync, __global int *plain) {

}
