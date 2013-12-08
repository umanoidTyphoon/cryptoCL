// MD5 leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))
#define BYTE_WITH_MSB_EQUAL_TO_ONE 1
#define HASH_SIZE 16
#define MD5_MSG_SIZE 56
#define PASSWD_FOUND 1
/* Number of zeroes attached to the password. The number depends
 * on the password length, since the message size has to reach
 * 448 (mod 512) bits, namely 56B. See MD5 simple implementation
 * on Wikipedia for further information:
 * 			    http://en.wikipedia.org/wiki/MD5
 */
#define ZEROES_PLUS_PASSLEN MD5_MSG_SIZE - BYTE_WITH_MSB_EQUAL_TO_ONE

/*
 * Set the components of an OpenCL vector of 16 unsigned chars
 * @param vec - the vector whose components have to be set
 * @param index - index of the vector component to be set
 * @param c - character to be set in the vector "vec" at the index "index"
 * @return void
 */
void setVecComponent(uchar16 vec, int index, uchar c){
	if	   (index==0)  vec.s0 = c;
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

/*
 * Check if the digest given in input to the kernel is equal to the digest
 * computed by the kernel
 * @param computedDigest - digest computed by the kernel
 * @param digest - digest given in input to the kernel
 * @return 0, if the two digest are different. Otherwise 1.
 */
int is_equal(int *computedDigest, int *digest){

	if(computedDigest[0] != digest[0] || computedDigest[1] != digest[1] ||
	   computedDigest[2] != digest[2] || computedDigest[3] != digest[3])
	   return 0;

	return 1;
}

/*
 * is_equal vector version
 */
int is_equal_Vector(uchar16 computedDigest, uchar16 digest){

	if(digest.s0 != computedDigest.s0) return 0;
	if(digest.s1 != computedDigest.s1) return 0;
	if(digest.s2 != computedDigest.s2) return 0;
	if(digest.s3 != computedDigest.s3) return 0;
	if(digest.s4 != computedDigest.s4) return 0;
	if(digest.s5 != computedDigest.s5) return 0;
	if(digest.s6 != computedDigest.s6) return 0;
	if(digest.s7 != computedDigest.s7) return 0;
	if(digest.s8 != computedDigest.s8) return 0;
	if(digest.s9 != computedDigest.s9) return 0;
	if(digest.sA != computedDigest.sA) return 0;
	if(digest.sB != computedDigest.sB) return 0;
	if(digest.sC != computedDigest.sC) return 0;
	if(digest.sD != computedDigest.sD) return 0;
	if(digest.sE != computedDigest.sE) return 0;
	if(digest.sF != computedDigest.sF) return 0;

	return 1;
}

/*
 * Set all the components of an integer array to zero
 * @param array - array to set to zero
 * @param dim - array length
 * @return void
 */
void clean_int_array(int *array, int dim){
	int i;
	for(i=0; i<dim; i++)
		array[i] = 0;
}

/*
 * Copy the digest given in input to the kernel from global to private memory
 * @param digest - private memory destination
 * @param hash - global memory source
 * @return void
 */

void cpy_in_pvt_mem(int *digest, __global const int *hash){
	digest[0] = hash[0]; digest[1] = hash[1];
	digest[2] = hash[2]; digest[3] = hash[3];
}

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

/*
 * Set the nth byte of an integer. Useful to pack a string in a integer
 * @param integer - pointer to an array of integer which will contain the string
 * @param byte_index - byte index to set
 * @param c - char to set in the integer array at index "byte_index"
 * @return void
 */
void set_nth_byte(int *integer, int byte_index, uchar c){
	*integer |= (c << (byte_index * 8));
}

/* MD5 implementation in OpenCL. Instead of unsigned chars, integers are used */
int md5(int *password, int initial_len, int cindex, int ch0, int ch1, int ch2, int ch3) {

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

	int  offset;
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

	set_nth_byte(password+cindex, initial_len % sizeof(int), 0x80); // append the "1" bit; most significant bit is "first"

	cindex = MD5_MSG_SIZE / sizeof(int);

	// append the len in bits at the end of the buffer.
	set_nth_byte(password+cindex, 0, (uchar) (initial_len * 8));
	set_nth_byte(password+cindex, 1, (uchar) ((initial_len * 8) >> 8 ));
	set_nth_byte(password+cindex, 2, (uchar) ((initial_len * 8) >> 16));
	set_nth_byte(password+cindex, 3, (uchar) ((initial_len * 8) >> 24));
	cindex++;
	set_nth_byte(password+cindex, 0, (uchar) (initial_len >> 29));
	set_nth_byte(password+cindex, 1, (uchar) ((initial_len >> 29) >> 8 ));
	set_nth_byte(password+cindex, 2, (uchar) ((initial_len >> 29) >> 16));
	set_nth_byte(password+cindex, 3, (uchar) ((initial_len >> 29) >> 24));

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
	return ((h0 == ch0) && (h1 == ch1) && (h2 == ch2) && (h3 == ch3));

}

void hash_cpy(__global int *dst, int *src){
	int i;
	for(int i=0; i<(HASH_SIZE/sizeof(int)); i++)
		dst[i] = src[i];
}

/* This function returns 0, if there's no next combination to generate. Otherwise, return 1                     */
int compute_new_starting_point(int *start_pt, int passlen, int cs_size){
	int i,pos;
	for(i=passlen-1; i>=0; i--){
		pos = start_pt[i] + 1;
		if(pos == cs_size){
		   start_pt[i] = 0;
		}
		else{
		   start_pt[i]++;
		   break;
		}
	}
	for(i=0; i<passlen; i++){
		if(start_pt[i] != 0)
		   return 1;
	}
	return 0;
}

//#define __kernel
//#define __global

__kernel void GPU_crackMD5(__global long *chunk, __global const int *hash, __global const char *cs,
						   __global int *cs_size, __global int *passlen,__global int *sync, __global int *plain,
						   __global unsigned char *computed_hash) {

	int i,j,k,count,cindex,cs_size_rv,equalityCheck,passlen_rv, pos,term;
	int digest[4], computedDigest[4];
	int h0, h1, h2, h3, out_h0, out_h1, out_h2, out_h3, sync_rv;
	// ASSUMPTION: 512 is the maximum length of a password which can be given in input to this kernel
	int password[512], start_pt[512];
	long chunk_rv, init, powl;
	int pass = 0;
	uchar alph[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
	int bitmap[80] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	// Get the index of the current element to be processed
	i = get_global_id(0);
	chunk_rv = *chunk;
	cs_size_rv = *cs_size;
	passlen_rv  = *passlen;
	// Conditional flag set after md5 computation and verification
	sync_rv = 0;
	out_h0 = 0; out_h1 = 0; out_h2 = 0; out_h3 = 0;

	clean_int_array(password, 512);
	clean_int_array(start_pt, 512);
	//cpy_in_pvt_mem(digest, (__global const int *) hash);
	h0 = ((__global const int *) hash)[0];
	h1 = ((__global const int *) hash)[1];
	h2 = ((__global const int *) hash)[2];
	h3 = ((__global const int *) hash)[3];

	/* --------------------------------------- Starting point computation ---------------------------------------- */
	init = chunk_rv * i;
	/* ------------------------------------------------------------------------------------------------------------ */

	for(j=0; j<chunk_rv; j++, init++){
		// Variable that stores the info about the next byte where save the passwords' chars
		count = 0;
		// Index of the integer array password
		cindex = 0;
		clean_int_array(password, 512);

		/* -------------------------- Compute the password string putting it in a Integer ------------------------- *
		 * This because ATI Mobility Radeon HD 4000 Series doesn't support access operations using data type sizes
		 * 											less than 32 bits 												*/
		for(k=0; k<passlen_rv; k++){
			pos = (int)((init/ipowl((long) (cs_size_rv), k)) % cs_size_rv);
			uchar c = (uchar)(97 + pos);
			set_nth_byte(password+cindex, count, c);
			if(count == (sizeof(int) - 1)){
			   count = 0;
			   cindex++;
			}
			else
				count++;
			}
		/* -------------------------------------------------------------------------------------------------------- */

		/* ------------------------------------- MD5 computation & verification ----------------------------------- */
		equalityCheck = md5(password, passlen_rv, cindex, h0, h1, h2, h3);
		/* -------------------------------------------------------------------------------------------------------- */

		//A GPU-core has already found the password: hence, quit
		if(equalityCheck == PASSWD_FOUND){
		   *sync=equalityCheck;
		   hash_cpy(plain, password);
		   return;
		   }
		}
		/* -------------------------------------------------------------------------------------------------------- */
		*sync = 0;
}


