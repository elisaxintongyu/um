/*
 *     SemMem.c
 *     by Elisa and Cynthia, 04/05/2025
 *     Project 6 - um
 *
 *     This class implements the definition of the memthods for the SegMem class
 */

#include "SegMem.h"

struct SegMem_T {
        unsigned curr_id;
        Seq_T empty_id;
        Seq_T memory;
};

/* initialize_seg
*
* Initialize the struct SegMem_T, and initialize the structure of segmented 
* memory which is represented by sequence of sequences
*
* Parameters:
*      FILE *instructions:    The input file with instructions in it
*
* Returns: an initialized struct SegMem_T
* Expects: None
*
* Notes: Allocates new memory for Seq_T (sequence of sequences); memory will 
* be deallocated when finishing using the segmented memory by calling the 
* seg_free() or deleting a segment by calling unmap_seg()
*/
SegMem_T initialize_segmem()
{
        SegMem_T seg_mem = malloc(sizeof(*seg_mem));
        assert(seg_mem != NULL);
        seg_mem->curr_id = 0;
        seg_mem->empty_id = Seq_new(0);
        seg_mem->memory = Seq_new(0);
        assert(seg_mem->memory != NULL);
        assert(seg_mem->empty_id != NULL);
        return seg_mem;
}

/* populate_seg
*
* Initialize $m[0] with the instructions read from file. 
*
* Parameters:
*      FILE *instructions:    The input file with instructions in it
*
* Returns: None
* Expects: None
*
* Notes: Allocates new memory for Seq_T (sequence of sequences); memory will 
* be deallocated when finishing using the segmented memory by calling the 
* seg_free() or deleting a segment by calling unmap_seg()
*/
void populate_seg(SegMem_T seg_mem, FILE *instructions)
{
        assert(seg_mem != NULL);
        uint32_t words = 0;
        int ch = 0;
        Seq_T seg0 = Seq_new(0);
        while ((ch = getc(instructions)) != EOF) {
                words = Bitpack_newu(0, 8, 24, ch);
                for (int i = 0; i < 3; i++) {
                        ch = getc(instructions);
                        words = Bitpack_newu(words, 8, 16 - (i * 8), ch);
                }
                Seq_addhi(seg0, (void *)(uintptr_t)words);
        }
        Seq_addhi(seg_mem->memory, seg0);
}

/* map_seg
*
* Initialize and create a new segment with the provided number of elements and 
* add the new segment to the segmented memory. Return new segid. Updates 
* curr_id. This function will be used when opcode Map Segment is used, which 
* communicates with um.h
*
* Parameters:
*      SegMem_T seg_mem:      The segmented memory to be updated. 
*      unsigned num_words:        number of words in the new segment.
*
* Returns: the id of the new segment created.
* Expects: the passed in SegMem_T is not NULL.
*
* Notes: It’s a CRE for seg_mem to be null.
*/
unsigned map_seg(SegMem_T seg_mem, unsigned num_words) 
{
        assert(seg_mem != NULL);
        /* initialize new segment */
        Seq_T new_seg = Seq_new(num_words);
        /* initialize elements to 0 */
        for (int i = 0; i < (int)num_words; i++) {
                Seq_addhi(new_seg, (void *)(uintptr_t)0);
        }
        assert(new_seg != NULL);
        /* check if there is an empty segment */
        if (Seq_length(seg_mem->empty_id) > 0) {
                unsigned empty_index 
                                = (uintptr_t)Seq_get(seg_mem->empty_id, 0);
                Seq_T seg = Seq_get(seg_mem->memory, empty_index);
                Seq_free(&seg);
                Seq_put(seg_mem->memory, empty_index, new_seg);
                Seq_remlo(seg_mem->empty_id);
                return empty_index;
        } else {
                Seq_addhi(seg_mem->memory, new_seg);
                seg_mem->curr_id++;
                return seg_mem->curr_id;
        }
}

/* unmap_seg
*
* Delete and Deallocate $m[index] and update the segmented memory represented by
* sequence of sequences. This function will be used when the opcode Unmap 
* Segment is used which communicates with um.h
*
* Parameters:
*      SegMem_T seg_mem:		The segmented memory to be updated
*      unsigned index:			The index of the segment to be updated
*
* Returns: None
* Expects: the seg_mem cannot be NULL
*
* Notes: 
* CRE if the seg_mem is NULL
* this function deallocates the memory of the unmapped segment when the opcode 
* Unmap Segment is used
*/
void unmap_seg(SegMem_T seg_mem, unsigned index)
{
        assert(seg_mem != NULL);
        Seq_addlo(seg_mem->empty_id, (void *)(uintptr_t)index);
}

/* seg_load
*
* Return the value stored in $m[segid][offset], which communicates with um.h 
* when the op code Segmented Load is used. 
*
* Parameters:
*      SegMem_T seg_mem:		The segmented memory to be updated
*      unsigned segid:			The id of the segment to be accessed
*      unsigned offset: 		The offset in the segment of the value 
                                        to be accessed
*
* Returns: The value (uint32_t) stored in the designated segment and offset. 
* Expects: The seg_mem cannot be NULL, segid cannot refer to an empty segment 
* or out ofrange, and offset can not be longer than the length of the segment 
* accessed.
*
* Notes: It’s a CRE for seg_mem to be null or any of segid or offset to be 
* accessing empty segment or out of range.
*/
uint32_t seg_load(SegMem_T seg_mem, unsigned segid, unsigned offset)
{
        //TESTING: print empty id and curr_id
        //printf("empty id: %u, curr_id: %u\n", (unsigned)(uintptr_t)Seq_get(seg_mem->empty_id, 0), seg_mem->curr_id);
        assert(seg_mem != NULL);
        Seq_T seg = Seq_get(seg_mem->memory, segid);
        assert(seg != NULL);
        /* get the value at the offset in the segment */
        uint32_t value = (uintptr_t)Seq_get(seg, offset);
        return value;
}

/* seg_store
*
* Update a word in the segmented memory, which is an element of the inner 
* sequence. This communicates with um.h when the op code Segmented Store is 
* used.
*
* Parameters:
*      SegMem_T seg_mem:        The segmented memory to be updated
*      unsigned segid:		The id of the segment to be updated
*      unsigned offset:		The offset of word to be updated to locate word
*      uint32_t value:		The new value to replace the original word
*
* Returns: the original value before the update
* Expects: The seg_mem cannot be NULL, segid cannot refer to an empty segment 
* or out of range, and offset can not be longer than the length of the segment 
* accessed.
*
* Notes: 
* CRE if seg_mem is NULL or any of segid or offset to be accessing empty 
* segment or out of range.
*/
uint32_t seg_store(SegMem_T seg_mem, unsigned segid, 
                        unsigned offset, uint32_t value) 
{
        assert(seg_mem != NULL);
        uint32_t old_value = (uint32_t)(uintptr_t)
                            Seq_get(Seq_get(seg_mem->memory, segid), offset);
        Seq_put(Seq_get(seg_mem->memory, segid), offset, 
                (void *)(uintptr_t)value);
        return old_value;
}

/* seg_free
*
* Deallocate the whole segmented memory when finished using
*
* Parameters:
*      SegMem_T seg_mem:		The segmented memory to be freed
*
* Returns: None
* Expects: The seg_mem cannot be NULL
*
* Notes: 
* CRE if seg_mem is NULL
* this function deallocates the memory of the segmented memory
*/
void seg_free(SegMem_T seg_mem)
{
        assert(seg_mem != NULL);

        /* free the mapped segments */
        for (int i = 0; i < Seq_length(seg_mem->memory); i++) {
                Seq_T seg = Seq_get(seg_mem->memory, i);
                Seq_free(&seg);
        }

        Seq_free(&seg_mem->memory);
        Seq_free(&seg_mem->empty_id);
        free(seg_mem);
}

/* seg_length
*
* Return the length of the segment with segid
*
* Parameters:
*      SegMem_T seg_mem:	The segmented memory to be updated
*      unsigned segid:		The id of the segment to be updated
*
* Returns: the length of the segment with segid
* Expects: The seg_mem cannot be NULL, segid cannot refer to an empty segment
* or out of range
*
* Notes:
* CRE if seg_mem is NULL or any of segid or offset to be accessing empty
* segment or out of range.
*/
int seg_length(SegMem_T seg_mem, unsigned segid)
{
        assert(seg_mem != NULL);
        return Seq_length(Seq_get(seg_mem->memory, segid));
}