/*
 *     um.c
 *     by Elisa and Cynthia, 04/10/2025
 *     Project 6 - um
 *
 *     This is the implementation of the um class. 
 *     It contains a segmented memory, a set of registers, and program counter. 
 *     This class also handles I/O operations, such as reading from input 
 *     and writing to output.
 */

#include "um.h"
#include "seq.h"
#include <bitpack.h>
#include <assert.h>
#include "SegMem.h"
#include <math.h>

/* declare private functions */
static inline void loadp_helper(uint32_t rb, uint32_t rc, UM_T um);
static inline void input_helper(unsigned c, UM_T um);
static inline void decode_execute(UM_T um, uint32_t instruction, bool *halt);

/* declare the um struct */
struct UM_T {
	int program_counter; 
	Seq_T registers; /* a sequence of 8 registers */
	SegMem_T seg_mem; /* segmented memory */
	FILE *input; /* input device */
	FILE *output; /* output device */
};

/* declare the opcodes, each represents a instruction */
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

/* declare constants */
const int REGISTERS = 8;
const int REGISTER_WIDTH = 3;
const int OPCODE_WIDTH = 4;
const int INSTRUCTION_WIDTH = 32;
const uint32_t MAX_VAL = 255;
const uint32_t OPCODE_NUM = 13;
const int VAL_WIDTH = 25;

/* new_um
*
* Initialize the UM struct by reading from the file
*
* Parameters:
*      FILE * instructions:		The input file with instructions in it
*      FILE* input:			the input stream used in I/O device
*      FILE* output:			the output stream used in I/O device
*
* Returns: An initialized UM struct
* Expects: The instructions cannot be NULL
*
* Notes: 
* CRE if instructions is NULL
* this function allocates the memory for the segmented memory, and registers, 
* which will be deallocated by calling the um_free after finishing executing 
* the program
*/
UM_T new_um(FILE *instructions, FILE *input, FILE *output)
{
        assert(instructions != NULL);

        UM_T um = malloc(sizeof(struct UM_T));
        assert(um != NULL);

        /* initialize the program counter */
        um->program_counter = 0;

        /* initialize the registers */
        um->registers = Seq_new(REGISTERS);
        assert(um->registers != NULL);

        /* initialize the registers to 0 */
        for (int i = 0; i < REGISTERS; i++) {
                Seq_addhi(um->registers, (void *)(uintptr_t)0);
        }

        /* initialize the segmented memory */
        um->seg_mem = initialize_segmem();
        assert(um->seg_mem != NULL);
        populate_seg(um->seg_mem, instructions);

        /* initialize the input and output streams */
        um->input = input;
        um->output = output;
        assert(um->input != NULL);
        assert(um->output != NULL);
        
        return um;
}

/* fetch_decode_execute
*
* Executes the program stored in $m[0]. Communicates with the registers and the
* segmented memory using the functions defined in SegMem.h and registers.h. 
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
                uint32_t instruction = seg_load(um->seg_mem, 0, 
                                                um->program_counter);

                /* Increment program counter */
                um->program_counter++;

                /* Decode and execute instruction */
                decode_execute(um, instruction, &halt);

        }
}

/* decode_execute
*
* Helper functio that executes the instruction by retrieving the opcode and 
* registers from the instruction.
*
* Parameters:
*      UM um:		        The UM to be executed
*      uint32_t instruction:	The instruction to be executed
*      bool *halt:		A pointer to a boolean that indicates if the
*                               program should halt
*
* Returns: None
* Expects: expect the UM to not be NULL
*
* Notes: None. 
*/
static inline void decode_execute(UM_T um, uint32_t instruction, bool *halt)
{
        assert(halt != NULL);
        assert(um != NULL);
        /* Execute instruction */
        /* Retrieve opcode */
        uint32_t opcode = Bitpack_getu(instruction, OPCODE_WIDTH, 
                                       INSTRUCTION_WIDTH - OPCODE_WIDTH);
        assert(opcode <= OPCODE_NUM);
        
        /* Retrieve registers */
        if (opcode != OPCODE_NUM) {
                unsigned a = Bitpack_getu(instruction, REGISTER_WIDTH, 
                                          REGISTER_WIDTH * 2);
                unsigned b = Bitpack_getu(instruction, REGISTER_WIDTH, 
                                          REGISTER_WIDTH);
                unsigned c = Bitpack_getu(instruction, REGISTER_WIDTH, 0);
                uint32_t ra = (uintptr_t)Seq_get(um->registers, a);
                uint32_t rb = (uintptr_t)Seq_get(um->registers, b);
                uint32_t rc = (uintptr_t)Seq_get(um->registers, c);

                switch (opcode) {
                        case CMOV:
                                if (rc != 0) {
                                        Seq_put(um->registers, a, 
                                                (void *)(uintptr_t)rb);
                                }
                        break;
                        case SLOAD:{
                                uint32_t value = seg_load(um->seg_mem,
                                         rb, rc);
                                Seq_put(um->registers, a, 
                                        (void *)(uintptr_t)value);
                        break;
                        }
                        case SSTORE:
                                seg_store(um->seg_mem, 
                                        ra, rb, rc);
                        break;
                        case ADD: 
                                Seq_put(um->registers, a, 
                                        (void *)(uintptr_t)((rb + rc)));
                        break;
                        case MUL:
                                Seq_put(um->registers, a, 
                                        (void *)(uintptr_t)((rb * rc)));
                        break;
                        case DIV:
                                Seq_put(um->registers, a, 
                                        (void *)(uintptr_t)(rb / rc));
                        break;
                        case NAND:
                                Seq_put(um->registers, a, 
                                        (void *)(uintptr_t)(~(rb & rc)));
                        break;
                        case HALT:
                                *halt = true;
                        break;
                        case ACTIVATE:{
                                uint32_t segid = map_seg(um->seg_mem, rc);
                                Seq_put(um->registers, b, 
                                        (void *)(uintptr_t)segid);
                        break;
                        }
                        case INACTIVATE:
                                unmap_seg(um->seg_mem, rc);
                        break;
                        case OUT:
                                assert(rc <= MAX_VAL);
                                fprintf(um->output, "%c", (char)rc);
                        break;
                        case IN: 
                                input_helper(c, um);
                        break;
                        case LOADP:
                                loadp_helper(rb, rc, um);
                        break;
                }
        } else {
                /* load value case */
                uint32_t a = Bitpack_getu(instruction, REGISTER_WIDTH, 
                                          VAL_WIDTH);
                uint32_t val = Bitpack_getu(instruction, VAL_WIDTH, 0);
                Seq_put(um->registers, a, (void *)(uintptr_t)val);
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
* this function deallocates the memory of the segmented memory(SegMem_T) and 
* registers 
*/
void um_free(UM_T um)
{
        assert(um != NULL);
        Seq_free(&um->registers);
        seg_free(um->seg_mem);
        free(um);
        um = NULL;
}

/* input_helper
*
* a helper funcion that reads from the input stream and stores the value in the
* register
*
* Parameters:
*      UM um:		        The UM struct 
*      unsigned c:		The register to store the value
*
* Returns: None
* Expects: UM to be not NULL.
*
* Notes: None
*/
static inline void input_helper(unsigned c, UM_T um)         
{
        uint32_t value = 0;
        value = getc(um->input);
        assert(value <= MAX_VAL);
        if ((int)value == EOF) {
                Seq_put(um->registers, c, 
                        (void *)(uintptr_t)0xFFFFFFFF);
        } else {
                Seq_put(um->registers, c, 
                        (void *)(uintptr_t)value);
        }

}

/* loadp_helper
*
* a helper function that loads the program from the segmented memory and
* stores it in the register
*
* Parameters:
*      UM um:		        The UM struct to be freed
*      unsigned rb:		The register that has the segment id
*      unsigned rc:		The register to store the value of 
*                               the program counter
*
* Returns: None
* Expects: UM to be not NULL.
*
* Notes: None
*/
static inline void loadp_helper(uint32_t rb, uint32_t rc, UM_T um) 
{
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

}