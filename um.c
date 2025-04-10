/*
 *     um.c
 *     by Elisa and Cynthia, 04/05/2025
 *     Project 6 - um
 *
 *     This is the implementation of the um class. 
 */
#include "um.h"

struct UM_T {
	int program_counter; 
	Seq_T registers;
	SegMem_T seg_mem;
	FILE *input;
	FILE *output;
};

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

/* new_um
*
* Initialize the UM struct by reading from the file
*
* Parameters:
*      FILE * instructions:		The input file with instructions in it
*      FILE* input:				the input stream used in I/O device
*      FILE* output:			the output stream used in I/O device
*
* Returns: An initialized UM struct
* Expects: The instructions cannot be NULL
*
* Notes: 
* CRE if instructions is NULL
* this function allocates the memory for the segmented memory, and registers, which will be 
* deallocated by calling the um_free after finishing executing the program
*/
UM_T new_um(FILE *instructions, FILE *input, FILE *output)
{
        assert(instructions != NULL);
        UM_T um = malloc(sizeof(struct UM_T));
        assert(um != NULL);
        um->program_counter = 0;
        um->registers = Seq_new(8);
        assert(um->registers != NULL);
        for (int i = 0; i < 8; i++) {
                Seq_addhi(um->registers, (void *)(uintptr_t)0);
        }

        um->seg_mem = initialize_segmem();
        assert(um->seg_mem != NULL);
        populate_seg(um->seg_mem, instructions);

        um->input = input;
        um->output = output;
        assert(um->input != NULL);
        assert(um->output != NULL);
        
        return um;
}

/* fetch_decode_execute
*
* Executes the program stored in $m[0]. Communicates with the registers and the
* segmented
* memory using the functions defined in SegMem.h and registers.h. 
*
* Parameters:
*      UM um:		The UM to be executed
*
* Returns: None
* Expects: The UM cannot be NULL
*
* Notes: 
* CRE if UM is NULL
* this function deallocates the memory of the segmented memory
*/
void fetch_decode_execute(UM_T um)
{
        assert(um != NULL);
        assert(um->seg_mem != NULL);
        assert(um->registers != NULL);

        bool halt = false;

        while (!halt) {
                /* Retrieve instruction */
                uint32_t instruction = seg_load(um->seg_mem, 0, um->program_counter);

                /* Increment program counter */
                um->program_counter++;

                /* Execute instruction */
                /* Retrieve opcode */
                uint32_t opcode = Bitpack_getu(instruction, 4, 28);
                assert(opcode <= 13);
                
                /* Retrieve registers */
                if (opcode != 13) {
                        unsigned a = Bitpack_getu(instruction, 3, 6);
                        unsigned b = Bitpack_getu(instruction, 3, 3);
                        unsigned c = Bitpack_getu(instruction, 3, 0);
                        uint32_t ra = (uintptr_t)Seq_get(um->registers, a);
                        uint32_t rb = (uintptr_t)Seq_get(um->registers, b);
                        uint32_t rc = (uintptr_t)Seq_get(um->registers, c);

                        switch (opcode) {
                                case CMOV:
                                        if (rc != 0) {
                                                Seq_put(um->registers, a, (void *)(uintptr_t)rb);
                                        }
                                break;
                                case SLOAD:{
                                        uint32_t value = seg_load(um->seg_mem, rb, rc);
                                        Seq_put(um->registers, a, (void *)(uintptr_t)value);
                                break;
                                }
                                case SSTORE:
                                        seg_store(um->seg_mem, ra, rb, rc);
                                break;
                                case ADD: 
                                        Seq_put(um->registers, a, (void *)(uintptr_t)((rb + rc)));
                                break;
                                case MUL:
                                        Seq_put(um->registers, a, (void *)(uintptr_t)((rb * rc)));
                                break;
                                case DIV:
                                        Seq_put(um->registers, a, (void *)(uintptr_t)(rb / rc));
                                break;
                                case NAND:
                                        Seq_put(um->registers, a, (void *)(uintptr_t)(~(rb & rc)));
                                break;
                                case HALT:
                                        halt = true;
                                break;
                                case ACTIVATE:{
                                        uint32_t segid = map_seg(um->seg_mem, rc);
                                        Seq_put(um->registers, b, (void *)(uintptr_t)segid);
                                break;
                                }
                                case INACTIVATE:
                                        unmap_seg(um->seg_mem, rc);
                                break;
                                case OUT:
                                        assert(rc <= 255);
                                        fprintf(um->output, "%c", (char)rc);
                                break;
                                case IN: {
                                        uint32_t value = 0;
                                        value = getc(um->input);
                                        assert(value <= 255);
                                        if ((int)value == EOF) {
                                                Seq_put(um->registers, c, (void *)(uintptr_t)0xFFFFFFFF);
                                        } else {
                                                Seq_put(um->registers, c, (void *)(uintptr_t)value);
                                        }
                                        break;
                                }
                                case LOADP:
                                        if (rb != 0) {
                                                int length = seg_length(um->seg_mem, rb);
                                                unmap_seg(um->seg_mem, 0);
                                                map_seg(um->seg_mem, length);
                                                for (int i = 0; i < length; i++) {
                                                        uint32_t temp_ins = seg_load(um->seg_mem, rb, i);
                                                        seg_store(um->seg_mem, 0, i, temp_ins);
                                                }
                                        }
                                        um->program_counter = rc;
                                break;
                        }
                } else {
                        /* load value case */
                        uint32_t a = Bitpack_getu(instruction, 3, 25);
                        uint32_t val = Bitpack_getu(instruction, 25, 0);
                        Seq_put(um->registers, a, (void *)(uintptr_t)val);
                }
        }
}

/* um_free
*
* Deallocate um when finished executing
*
* Parameters:
*      UM um:		The UM struct to be freed
*
* Returns: None
* Expects: UM to be not NULL.
*
* Notes: 
* this function deallocates the memory of the segmented memory(SegMem_T) and registers 
* (USeq_T)
*/
void um_free(UM_T um)
{
        assert(um != NULL);
        Seq_free(&um->registers);
        seg_free(um->seg_mem);
        free(um);
        um = NULL;
}
