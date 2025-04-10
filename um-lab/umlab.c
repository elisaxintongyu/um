/*
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>


typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
{
        assert(op <= 0xF);
        assert(ra <= 0x7);
        assert(rb <= 0x7);
        assert(rc <= 0x7);
        return Bitpack_newu(0, 4, 28, op) |
               Bitpack_newu(0, 3, 6, ra) |
               Bitpack_newu(0, 3, 3, rb) |
               Bitpack_newu(0, 3, 0, rc);
}

Um_instruction loadval(unsigned ra, unsigned val)
{
        assert(ra <= 0x7);
        assert(val <= 0xFFFFFF);
        return Bitpack_newu(0, 4, 28, LV) |
               Bitpack_newu(0, 3, 25, ra) |
               Bitpack_newu(0, 25, 0, val);
}


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction conditional_move(Um_register a, Um_register b, Um_register c) 
{
        return three_register(CMOV, a, b, c);
}

static inline Um_instruction sload(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SLOAD, a, b, c);
}
static inline Um_instruction sstore(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SSTORE, a, b, c);
}
static inline Um_instruction multiply(Um_register a, Um_register b, Um_register c) 
{
        return three_register(MUL, a, b, c);
}
static inline Um_instruction divide(Um_register a, Um_register b, Um_register c) 
{
        return three_register(DIV, a, b, c);
}
static inline Um_instruction nand(Um_register a, Um_register b, Um_register c) 
{
        return three_register(NAND, a, b, c);
}
static inline Um_instruction activate(Um_register b, Um_register c) 
{
        return three_register(ACTIVATE, 0, b, c);
}
static inline Um_instruction inactivate(Um_register c) 
{
        return three_register(INACTIVATE, 0, 0, c);
}
static inline Um_instruction input(Um_register c) 
{
        return three_register(IN, 0, 0, c);
}
static inline Um_instruction loadp(Um_register b, Um_register c) 
{
        return three_register(LOADP, 0, b, c);
}
static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

Um_instruction output(Um_register c)
{
        return three_register(OUT, 0, 0, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
      
}



/* Unit tests for the UM */

/* test only halt */ 
void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}
/* test loadval, output and halt */
void test_output(Seq_T stream)
{
    append(stream, loadval(r1, 65));    
    append(stream, output(r1));          /* output 'A'*/
    append(stream, halt());
}

/* test if successfully halt (not executing anything else), or prints "Bad!" */
void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

/* test conditional move when there's 0 in register c, so the content */
/* in register b will not be moved to a */
void cmov0_test(Seq_T stream)
{
        append(stream, loadval(r1, 58));
        append(stream, loadval(r2, 2));
        append(stream, loadval(r3, 0));
        append(stream, conditional_move(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

/* test conditional move when there's 1 in register c, so the content */
/* in register b will be moved to a */
void cmov1_test(Seq_T stream)
{
        append(stream, loadval(r1, 58));
        append(stream, loadval(r2, 59));
        append(stream, loadval(r3, 1));
        append(stream, conditional_move(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

/* test arith */
/* test add with a working loadval, output, and halt */
void add_test(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* test multiply */
void multiply_test(Seq_T stream)
{
        append(stream, loadval(r1, 7));
        append(stream, loadval(r2, 8));
        append(stream, multiply(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* test divide */
void divide_test(Seq_T stream)
{
        append(stream, loadval(r1, 99));
        append(stream, loadval(r2, 3));
        append(stream, divide(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* test nand */
void nand_test(Seq_T stream)
{
        append(stream, nand(r1, r2, r2));
        append(stream, loadval(r2, 33));
        append(stream, nand(r3, r2, r2));
        append(stream, nand(r3, r3, r1));
        append(stream, output(r3));
        append(stream, halt());
}

/* test combining all the arith */
void arith_test(Seq_T stream)
{
        /* Load r1 = 33, r2 = 3 */
        append(stream, loadval(r1, 33));                /* r1 = 33 */ 
        append(stream, loadval(r2, 3));                 /* r2 = 3 */

        /* multiply */
        append(stream, multiply(r3, r1, r2));             /* r3 = 99 */ 
        append(stream, output(r3));               /* Output 'c' (ASCII 99) */

        /* divide */
        append(stream, divide(r3, r3, r2));               /* r3 = 33 */ 
        append(stream, output(r3));               /* Output '!' (ASCII 33) */

        /* add */
        append(stream, add(r3, r3, r2));                  /* r3 = 36 */
        append(stream, output(r3));               /* Output '$' (ASCII 36) */

        /* nand */
        append(stream, nand(r3, r4, r4));
        append(stream, nand(r5, r1, r1));
        append(stream, nand(r5, r3, r5));
        append(stream, output(r5));
        append(stream, halt());                     /* Output '!' (ASCII 33) */

        append(stream, halt());
}

/* test activate */
void activate_test(Seq_T stream)
{
        append(stream, loadval(r3, 10));
        append(stream, activate(r2, r3));
        append(stream, loadval(r1, 58));
        append(stream, add(r1, r1, r2));
        append(stream, output(r2));
        append(stream, halt());
}

/* test sstore with a working activate function */
void test_sstore(Seq_T stream)
{
        append(stream, loadval(r1, 1));       
        append(stream, activate(r2, 1));      

        append(stream, loadval(r3, 65));      /* r3 = 'A' (ASCII 65) */
        append(stream, loadval(r4, 0));       

        append(stream, sstore(r2, r4, r3));    /* m[r2][0] = r3 */

        /* We don't output here, just confirm the store runs without failure */
        append(stream, halt());
}

/* test sload with a working sstore function */
void test_sload(Seq_T stream)
{
        append(stream, loadval(r1, 1));       
        append(stream, activate(r2, 1));      
    
        append(stream, loadval(r3, 65));      /* r3 = 'A' (ASCII 65) */
        append(stream, loadval(r4, 0));       
    
        append(stream, sstore(r2, r4, r3));    /* m[r2][0] = r3 */
        append(stream, sload(r5, r2, r4));     /* r5 = m[r2][0] */
        append(stream, output(r5));                /* output 'A' */
        append(stream, halt());
}

/* test inactivate with a working sstore and sload */
void inactivate_test(Seq_T stream)
{
        append(stream, loadval(r1, 33));
        append(stream, activate(r2, r1));
        append(stream, loadval(r3, 58));
        append(stream, loadval(r4, 0));
        append(stream, sstore(r2, r4, r3));

        append(stream, inactivate(r2));

        append(stream, activate(r2, r1));
        append(stream, loadval(r6, 66));
        append(stream, sstore(r2, r4, r6));
        append(stream, sload(r7, r2, r4));
        append(stream, output(r7)); /* should print B (ASCII 66) */
        append(stream, halt());
}

/* test loadp */
void loadp_test(Seq_T stream)
{
        append(stream, loadval(r1, 58));
        append(stream, loadval(r2, 10));
        append(stream, loadval(r3, 0));
        append(stream, sstore(r3, r2, r1));
        append(stream, loadp(r2, r3));
        append(stream, output(r2));
        append(stream, halt());
}

/* test activate, sload, and sstore */
void seg_test(Seq_T stream)
{
        append(stream, loadval(r1, 58));
        append(stream, loadval(r3, 10));
        append(stream, activate(r2, r3));
        append(stream, sstore(r2, r2, r1));
        append(stream, sload(r2, r2, r2));
        append(stream, output(r2));
        append(stream, halt());
}

/* test adding itself and input */
void times_two_test(Seq_T stream)
{
        append(stream, input(r3));
        append(stream, add(r4, r3, r3));
        append(stream, output(r4));
        append(stream, halt());
}

/* test mutiplly and input */
void times_three_test(Seq_T stream)
{
        append(stream, input(r3));
        append(stream, loadval(r2, 3));
        append(stream, multiply(r4, r2, r3));
        append(stream, output(r4));
        append(stream, halt());
}
void one_million_test(Seq_T stream)
{
        append(stream, loadval(r1, 1));
        for (int i = 0; i < 999998; i++) {
                append(stream, add(r2, r1, r2));
        }
        append(stream, halt());
}