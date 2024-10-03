#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "mem_alloc_types.h"
#include "mem_alloc_standard_pool.h"
#include "my_mmap.h"
#include "mem_alloc.h"

/////////////////////////////////////////////////////////////////////////////

#ifdef STDPOOL_POLICY
/* Get the value provided by the Makefile */
std_pool_placement_policy_t std_pool_policy = STDPOOL_POLICY;
#else
std_pool_placement_policy_t std_pool_policy = DEFAULT_STDPOOL_POLICY;
#endif

/////////////////////////////////////////////////////////////////////////////

mem_std_free_block_t *current = NULL;

void init_standard_pool(mem_pool_t *p, size_t size, size_t min_request_size, size_t max_request_size)
{
    void *address = my_mmap(size);
    if (address == NULL)
    {
        perror("Memory allocation failed for the standard pool");
        return;
    }

    // Initialize the pool
    p->start_addr = address;
    p->end_addr = (void *)((char *)address + size);
    p->pool_size = size;
    p->min_req_size = min_request_size;
    p->max_req_size = max_request_size;
    p->first_free = address;

    //First free block    
    mem_std_free_block_t *first_block = (mem_std_free_block_t *)address;

    /*
    Variable used in mem_alloc_standard_pool() as a reference to the last looked at free block
    It is initialized there so that it can be accessed staticly by the fuction
    Be careful may be prone to bugs
    */
    current = first_block;

    // Set the block size and mark it as free
    set_block_size(&(first_block->header), size - sizeof(mem_std_block_header_footer_t) * 2); // Remove header and footer
    set_block_free(&(first_block->header));

    first_block->next = NULL; // No next free block yet
    first_block->prev = NULL; // No previous free block yet

    // Footer matches the header
    mem_std_block_header_footer_t *first_footer = (mem_std_block_header_footer_t *)((char *)address + size - sizeof(mem_std_block_header_footer_t));
    *first_footer = first_block->header;

    printf("Standard pool initialized with a block of size %zu bytes\n", get_block_size(&(first_block->header)));
}

void *mem_alloc_standard_pool(mem_pool_t *pool, size_t size){
    
    //booleans to evaluate if the blocks can acomodate the alocation
         

    /*
    this switch is used to defin which is the best block according
    to the chosen placement policie and assign it to current
    */
    switch (std_pool_policy){
    case FIRST_FIT:
        // Traverse the free list from the beggining to find the first fit block
        current = (mem_std_free_block_t *)pool->first_free;
        while (current != NULL){
            char is_free = is_block_free(&(current->header));
            char is_sufficently_big = get_block_size(&(current->header)) >= size; 
            if (is_free && is_sufficently_big){
                break; // Found a suitable block
            }
            current = current->next;
        }
        break;

    case BEST_FIT:

        // Traverse the free list to find the block whose size is closest to the allocated size
        current = (mem_std_free_block_t *)pool->first_free;
        mem_std_free_block_t *best=current;
        while (current != NULL){
            char is_free = is_block_free(&(current->header));
            char is_sufficently_big = get_block_size(&(current->header)) >= size; 
            //evaluating if the current block is closer to the desired size than the precedent best (considering that it is already big enough)
            char is_best_fit = get_block_size(&(current->header)) < get_block_size(&(best->header));
            if (is_free && is_sufficently_big && is_best_fit){
                best = current;
            }
            current = current->next;  
        }
        current = best;
        break;

    case NEXT_FIT:

        while (current != NULL){
            char is_free = is_block_free(&(current->header));
            char is_sufficently_big = get_block_size(&(current->header)) >= size; 
            if (is_free && is_sufficently_big){
                break; // Found a suitable block
            }
            current = current->next;
            if(current == NULL){
                current = (mem_std_free_block_t *)pool->first_free;
            }
        }
        break;
    
    default:
        break;
    }
   

    // No suitable block found
    if (current == NULL)
    {
        printf("Error: No suitable block found in the standard pool\n");
        return NULL;
    }

    // If the block is larger than needed, split it
    size_t block_size = get_block_size(&(current->header));
    size_t total_size = size + sizeof(mem_std_block_header_footer_t) * 2;
    size_t remaining_size = block_size - size - sizeof(mem_std_block_header_footer_t) * 2;

    if (remaining_size >= sizeof(mem_std_free_block_t) + sizeof(mem_std_block_header_footer_t))
    {
        // There is enough space for a new block, so split
        mem_std_free_block_t *new_block = (mem_std_free_block_t *)((char *)current + total_size);

        // Set the size and mark the new block as free
        set_block_size(&(new_block->header), remaining_size);
        set_block_free(&(new_block->header));

        // Update the next and prev pointers for the new block
        new_block->next = current->next;
        new_block->prev = current;
        if (current->next != NULL)
        {
            current->next->prev = new_block;
        }
        current->next = new_block;

        // Set the footer for the new free block
        mem_std_block_header_footer_t *new_footer = (mem_std_block_header_footer_t *)((char *)new_block + remaining_size);
        *new_footer = new_block->header;

        // Adjust the current block size
        set_block_size(&(current->header), size);
    }

    // Mark the block as used
    set_block_used(&(current->header));
    mem_std_block_header_footer_t *footer = (mem_std_block_header_footer_t *)((char *)current + sizeof(mem_std_block_header_footer_t) + get_block_size(&(current->header)));
    *footer = current->header;

    // Return the memory address after the header
    return (void *)((char *)current + sizeof(mem_std_block_header_footer_t));
}

void mem_free_standard_pool(mem_pool_t *pool, void *addr)
{
    // Get the header of the block being freed by subtracting the size of the header
    mem_std_free_block_t *block = (mem_std_free_block_t *)((char *)addr - sizeof(mem_std_block_header_footer_t));

    // Mark the block as free
    set_block_free(&(block->header));

    // Get the block footer
    mem_std_block_header_footer_t *footer = (mem_std_block_header_footer_t *)((char *)block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(block->header)));
    *footer = block->header; // Update the footer to match the header

    // Coalesce with the next block if it's free
    mem_std_free_block_t *next_block = (mem_std_free_block_t *)((char *)footer + sizeof(mem_std_block_header_footer_t));
    if ((char *)next_block < (char *)pool->end_addr && is_block_free(&(next_block->header)))
    {
        // Merge current block with the next free block
        size_t new_size = get_block_size(&(block->header)) + sizeof(mem_std_block_header_footer_t) * 2 + get_block_size(&(next_block->header));
        set_block_size(&(block->header), new_size);
        set_block_free(&(block->header));

        // Update footer to match the new size
        footer = (mem_std_block_header_footer_t *)((char *)block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(block->header)));
        *footer = block->header;

        // Remove next block from the free list
        if (next_block->next)
        {
            next_block->next->prev = block;
        }
        block->next = next_block->next;
    }

    // Coalesce with the previous block if it's free
    mem_std_block_header_footer_t *prev_footer = (mem_std_block_header_footer_t *)((char *)block - sizeof(mem_std_block_header_footer_t));
    if ((char *)prev_footer > (char *)pool->start_addr && is_block_free(prev_footer))
    {
        // Get the previous block header
        mem_std_free_block_t *prev_block = (mem_std_free_block_t *)((char *)prev_footer - get_block_size(prev_footer) - sizeof(mem_std_block_header_footer_t));

        // Merge the previous block with the current block
        size_t new_size = get_block_size(&(prev_block->header)) + sizeof(mem_std_block_header_footer_t) * 2 + get_block_size(&(block->header));
        set_block_size(&(prev_block->header), new_size);
        set_block_free(&(prev_block->header));

        // Update footer of the merged block
        footer = (mem_std_block_header_footer_t *)((char *)prev_block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(prev_block->header)));
        *footer = prev_block->header;

        // Remove current block from the free list
        if (block->next)
        {
            block->next->prev = prev_block;
        }
        prev_block->next = block->next;

        block = prev_block; // Continue coalescing from the previous block
    }

    // Insert the freed (or coalesced) block back into the free list
    mem_std_free_block_t *current = (mem_std_free_block_t *)pool->first_free;
    mem_std_free_block_t *prev = NULL;

    while (current != NULL && (char *)current < (char *)block)
    {
        prev = current;
        current = current->next;
    }

    block->next = current;
    block->prev = prev;
    if (current != NULL)
    {
        current->prev = block;
    }
    if (prev != NULL)
    {
        prev->next = block;
    }
    else
    {
        pool->first_free = block; // This is the new head of the free list
    }
}

size_t mem_get_allocated_block_size_standard_pool(mem_pool_t *pool, void *addr)
{
    mem_std_allocated_block_t *block = (mem_std_allocated_block_t *)((char *)addr - sizeof(mem_std_block_header_footer_t));
    return get_block_size(&(block->header));
}
