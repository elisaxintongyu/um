/*
 *     SemMem.h
 *     by Elisa and Cynthia, 04/10/2025
 *     Project 6 - um
 *
 *     Declarations for the SegMem module, which cosists 
 *     the next available id, empty id list and the memory itself. This class 
 *     allows for modification and loading of each segments, as well as the 
 *     elements stored in it.
 */
#ifndef SEGMEM_INCLUDED 
#define SEGMEM_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <bitpack.h>

#define T SegMem_T
typedef struct T *T;


T initialize_segmem();

void populate_seg(T seg_mem, FILE *instructions);

unsigned map_seg(T seg_mem, unsigned num_words);

void unmap_seg(T seg_mem, unsigned index);

uint32_t seg_load(T seg_mem, unsigned segid, unsigned offset);

uint32_t seg_store(T seg_mem, unsigned segid, unsigned offset, uint32_t value);

void seg_free(T seg_mem);

int seg_length(T seg_mem, unsigned segid);

#undef T
#endif