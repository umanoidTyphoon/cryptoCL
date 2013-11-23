#define __kernel
#define __global

// leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))
/* Number of zeroes attached to the password. The number depends
 * on the password length, since the message size has to reach
 * 448 (mod 512) bits, namely 56B. See MD5 simple implementation
 * on Wikipedia for further information:
 * 			    http://en.wikipedia.org/wiki/MD5
 */
#define MD5_MSG_SIZE 56
#define BYTE_WITH_MSB_EQUAL_TO_ONE 1
#define ZEROES_PLUS_PASSLEN MD5_MSG_SIZE - BYTE_WITH_MSB_EQUAL_TO_ONE


void setVecComponent(uchar16 vec, int index, uchar c){
	if(index==0)  vec.s0 = c;
	else if(index==1)  vec.s1 = c;
	else if(index==2)  vec.s2 = c;
	else if(index==3)  vec.s3 = c;
	else if(index==4)  vec.s4 = c;
	else if(index==5)  vec.s5 = c;
	else if(index==6)  vec.s6 = c;
	else if(index==7)  vec.s7 = c;
	else if(index==8)  vec.s8 = c;
	else if(index==9)  vec.s9 = c;
	else if(index==10) vec.sA = c;
	else if(index==11) vec.sB = c;
	else if(index==12) vec.sC = c;
	else if(index==13) vec.sD = c;
	else if(index==14) vec.sE = c;
	else if(index==15) vec.sF = c;
}

void clean_int_array(int *array, int dim){
	int i;
	for(i=0; i<dim; i++)
		array[i] = 0;
}

void set_nth_byte(int integer, int byte_index, uchar c){
	integer |= (c << (byte_index * 8));
}

void md5(int *password, int initial_len, int cindex, uchar16 digest) {

	// Constants are the integer part of the sines of integers (in radians) * 2^32.
	const uint k[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee ,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 ,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be ,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 ,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa ,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 ,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed ,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a ,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c ,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 ,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 ,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 ,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 ,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 ,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 ,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

	// r specifies the per-round shift amounts
	const uint r[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	                      5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
	                      4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	                      6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

	// These vars will contain the hash
	uint h0, h1, h2, h3;

	int count, new_len, offset, zeroes;
	uint w[16];
	uint a, b, c, d, i, f, g, temp;

	// Initialize variables - simple count in nibbles:
	h0 = 0x67452301;
	h1 = 0xefcdab89;
	h2 = 0x98badcfe;
	h3 = 0x10325476;

	//Pre-processing:
	//append "1" bit to message
	//append "0" bits until message length in bits ≡ 448 (mod 512) UNUSEFUL since my array is initialized to 0
	//append length mod (2^64) to message

	set_nth_byte(password[cindex], initial_len % sizeof(int), 0x80); // append the "1" bit; most significant bit is "first"

	zeroes = MD5_MSG_SIZE - BYTE_WITH_MSB_EQUAL_TO_ONE - initial_len;
	cindex += (zeroes >> sizeof(int)) + 1;

	// append the len in bits at the end of the buffer.
	set_nth_byte(password[cindex], 0, (uchar) (initial_len * 8));
	set_nth_byte(password[cindex], 1, (uchar) ((initial_len * 8) >> 8 ));
	set_nth_byte(password[cindex], 2, (uchar) ((initial_len * 8) >> 16));
	set_nth_byte(password[cindex++], 3, (uchar) ((initial_len * 8) >> 24));

	set_nth_byte(password[cindex], 0, (uchar) (initial_len >> 29));
	set_nth_byte(password[cindex], 1, (uchar) ((initial_len >> 29) >> 8 ));
	set_nth_byte(password[cindex], 2, (uchar) ((initial_len >> 29) >> 16));
	set_nth_byte(password[cindex], 3, (uchar) ((initial_len >> 29) >> 24));

	// Process the message in successive 512-bit chunks:
	//for each 512-bit chunk of message:
	for(offset=0; offset<MD5_MSG_SIZE; offset += (512/8)) {

		// break chunk into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
		for (i = 0; i < 16; i++)
			 w[i] = password[i];

		// Initialize hash value for this chunk:
		a = h0;
		b = h1;
		c = h2;
		d = h3;

		// Main loop:
		for(i = 0; i<64; i++) {

			if (i < 16) {
				f = (b & c) | ((~b) & d);
				g = i;
			} else if (i < 32) {
				f = (d & b) | ((~d) & c);
				g = (5*i + 1) % 16;
			} else if (i < 48) {
				f = b ^ c ^ d;
				g = (3*i + 5) % 16;
			} else {
				f = c ^ (b | (~d));
				g = (7*i) % 16;
			}

			temp = d;
			d = c;
			c = b;
			b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
			a = temp;

		}

		// Add this chunk's hash to result so far:
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
	}

	//var char digest[16] := h0 append h1 append h2 append h3 //(Output is in little-endian)
	digest.s0 = (uchar)  h0;
	digest.s1 = (uchar) (h0 >> 8);
	digest.s2 = (uchar) (h0 >> 16);
	digest.s3 = (uchar) (h0 >> 24);
	digest.s4 = (uchar)  h1;
	digest.s5 = (uchar) (h1 >> 8);
	digest.s6 = (uchar) (h1 >> 16);
	digest.s7 = (uchar) (h1 >> 24);
	digest.s8 = (uchar)  h2;
	digest.s9 = (uchar) (h2 >> 8);
	digest.sA = (uchar) (h2 >> 16);
	digest.sB = (uchar) (h2 >> 24);
	digest.sC = (uchar)  h3;
	digest.sD = (uchar) (h3 >> 8);
	digest.sE = (uchar) (h3 >> 16);
	digest.sF = (uchar) (h3 >> 24);
}

__kernel void GPU_crackMD5(__global long *chunk, __global const unsigned char *hash, __global const char *cs,
						   __global int *cs_size, __global int *passlen,__global int *sync, __global char *plain,
						   __global unsigned char *computed_hash) {

	int i,j,k,count,cindex,pos;
	long init, powl;
	// ASSUMPTION: 512 is the maximum length of a password which can be given in input to this kernel
	int start_pt[512];
	int password[512];
	uchar16 digest;

	// get the index of the current element to be processed
	i = get_global_id(0);
	clean_int_array(password, 512);
	clean_int_array(start_pt, 512);

	/* --------------------------------------- Starting point computation ---------------------------------------- */
	init = *chunk * i;
	powl = *cs_size;
	//int *start_pt = compute_starting_point(init, cs_size, passlen);
	for(k=0; k<*passlen; k++){
		powl = (powl << (*cs_size/k)) + ((powl % k) * powl);
		pos = (int)(init/(powl % *cs_size));
		start_pt[*passlen - k - 1] = pos;
	}
	/* ------------------------------------------------------------------------------------------------------------ */

	// Variable that stores the info about the next byte where save the passwords' chars
	count = 0;
	// Index of the integer array password
	cindex = 0;
	for(j=0; j<*chunk; j++){
		/* -------------------------- Compute the password string putting it in a Integer ------------------------- *
		 * This because ATI Mobility Radeon HD 4000 Series doesn't support access operations using data type sizes
		 * 											less than 32 bits 												*/
		for(k=0; k<*passlen; k++){
			uchar c = (uchar)(cs[start_pt[k]]);
			set_nth_byte(password[cindex], count, c);
			if(count == (sizeof(int) - 1)){
			   count = 0;
			   cindex++;
			}
			else
				count++;
		}
		/* -------------------------------------------------------------------------------------------------------- */

		/* --------------------------------------------- MD5 computation ------------------------------------------ */
		md5(password, *passlen, cindex, digest);
		/* -------------------------------------------------------------------------------------------------------- */
	}
}
