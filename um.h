/*
 *     um.h
 *     by Elisa and Cynthia, 04/05/2025
 *     Project 6 - um
 *
 *     This is the implementation of the um class. 
 */
#ifndef UM_INCLUDED 
#define UM_INCLUDED

#include <stdlib.h>
#include "seq.h"
#include <bitpack.h>
#include <stdio.h>
#include <assert.h>
#include "set.h"
#include "SegMem.h"

#include <math.h>

#define T UM_T
typedef struct T *T;

T new_um(FILE * instructions, FILE* input, FILE* output);

void fetch_decode_execute(T um);

void um_free(T um);

#undef T
#endif