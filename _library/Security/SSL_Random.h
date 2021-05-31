/*
 * file: SSL_Random.c
 * description: random generator function
 * author: peter
 * company: wiznet
 * data: 2015.11.26
 */


#ifndef SSL_Random
#define SSL_Random

#include <stdlib.h>

void RandomSeeding(int randSeed);
int SSLRandomCB( void *p_rng, unsigned char *output, size_t output_len );


#endif
