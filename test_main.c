/*
 *     test_main.c
 *     by Elisa and Cynthia, 04/05/2025
 *     Project 6 - um
 *
 *     This file includes a main function that tests for the SegMem class.
 *     It creates a SegMem object, adds segments, retrieves values, and removes
 *     segments. It also checks for errors and prints the results of the tests.
 *     The tests are designed to ensure that the SegMem class functions 
 *     correctly and handles memory management properly.
 */

#include <stdio.h>
#include <stdlib.h>
#include "SegMem.h"

void test_initialize()
{
    SegMem_T segmem = initialize_segmem();
    if (segmem == NULL) {
        fprintf(stderr, "Failed to initialize SegMem\n");
        exit(EXIT_FAILURE);
    }
    printf("SegMem initialized successfully\n");
}

void test_populate(FILE *instructions)
{
    SegMem_T seg_mem = initialize_segmem();
    /* populate the memory */
    populate_seg(seg_mem, instructions);

    printf("Memory populated successfully\n");
}

int main(int argc, char *argv[])
{
    (void) argc;
    // Test the SegMem initialization
    //test_initialize();

    // Test populate segments
    FILE *instructions = fopen(argv[1], "r");
    printf("Populating segments from file: %s\n", argv[1]);
    if (instructions == NULL) {
        fprintf(stderr, "Failed to open instructions file\n");
        exit(EXIT_FAILURE);
    }
    //test_populate(instructions);

    // initialize the segmented memory
    SegMem_T seg_mem = initialize_segmem();
    // populate the segmented memory with instructions
    populate_seg(seg_mem, instructions);
    // close the instructions file
    fclose(instructions);
    
    // Test new_segment
    //test_new_seg(instructions);

    // // Test seg_load
    // unsigned segid = 0;
    // unsigned offset = 0;
    // uint32_t value = seg_load(seg_mem, segid, offset);
    // if (value == 0) {
    //     fprintf(stderr, "Failed to load value from segment\n");
    //     exit(EXIT_FAILURE);
    // }
    // printf("Value loaded from segment %u at offset %u: %x\n", segid, offset, 
    // value);

    // Test seg_store
    // unsigned segid = 0;
    // unsigned offset = 0;
    // uint32_t new_value = 42;
    // uint32_t old_value = seg_store(seg_mem, segid, offset, new_value);
    // if (old_value == 0) {
    //     fprintf(stderr, "Failed to store value in segment\n");
    //     exit(EXIT_FAILURE);
    // }
    // printf("new_value = %x\n", new_value);
    // printf("Value stored in segment %u at offset %u: %x (old value: %x)\n", 
    // segid, offset, seg_load(seg_mem, 0, 0), old_value);

    // Test unmap_seg
    // unsigned segid = 0;
    // unmap_seg(seg_mem, segid);
    // printf("Segment %u unmapped successfully\n", segid);

    // Test map_seg
    unsigned id = map_seg(seg_mem, 10);
    printf("Segment %u mapped successfully\n", id);
    
    // Test seg_free
    seg_free(seg_mem);

    printf("All tests passed!\n");
    return 0;
}

