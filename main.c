/*
 *     main.c
 *     by Elisa and Cynthia, 04/05/2025
 *     Project 6 - um
 *
 *     This file includes a main function that initialize and execute the UM 
 *     class according the instructions stored in the provided files.
 */

#include <stdio.h>
#include <stdlib.h>
#include "um.h"

int main(int argc, char *argv[])
{
        if (argc != 2) {
                fprintf(stderr, "Usage: %s <instructions_file>\n", argv[0]);
                return EXIT_FAILURE;
        }

        FILE *instructions = fopen(argv[1], "r");

        if (instructions == NULL) {
                fprintf(stderr, "Error opening instruction file\n");
                return EXIT_FAILURE;
        }

        UM_T um = new_um(instructions, stdin, stdout);
        fetch_decode_execute(um);
        um_free(um);

        fclose(instructions);

        return EXIT_SUCCESS;
}