/*
 *     um.h
 *     by Elisa and Cynthia, 04/10/2025
 *     Project 6 - um
 *
 *     This is the header file for the um module.
 *     It contains the declaration of the um struct, and the methods that are
 *     used to manipulate the um struct, such as creating a new um,
 *     executing instructions, and freeing the um struct.
 */

#ifndef UM_INCLUDED 
#define UM_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <bitpack.h>

#define T UM_T
typedef struct T *T;

T new_um(FILE * instructions, FILE* input, FILE* output);

void fetch_decode_execute(T um);

void um_free(T um);

#undef T
#endif