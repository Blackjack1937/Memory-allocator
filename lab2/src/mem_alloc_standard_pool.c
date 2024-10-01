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

    // First free block
    mem_std_free_block_t *first_block = (mem_std_free_block_t *)address;

    // Set the block size and mark it as free
    set_block_size(&(first_block->header), size - sizeof(mem_std_block_header_footer_t) * 2); // Remove header and footer
    set_block_free(&(first_block->header));

    first_block->next = NULL; // No next free block yet
    first_block->prev = NULL; // No previous free block yet
    // Footer match the header
    mem_std_block_header_footer_t *first_footer = (mem_std_block_header_footer_t *)((char *)address + size - sizeof(mem_std_block_header_footer_t));
    *first_footer = first_block->header;

    printf("Standard pool initialized with a block of size %zu bytes\n", get_block_size(&(first_block->header)));
}

void *mem_alloc_standard_pool(mem_pool_t *pool, size_t size)
{
    mem_std_free_block_t *current = (mem_std_free_block_t *)pool->first_free;

    // Find the first fit block
    while (current != NULL)
    {
        if (is_block_free(&(current->header)) && get_block_size(&(current->header)) >= size)
        {
            break;
        }
        current = current->next;
    }
    if (current == NULL)
    {
        printf("Error: No suitable block found in the standard pool\n");
        return NULL;
    }

    // Split the block if too large
    size_t block_size = get_block_size(&(current->header));
    if (block_size > size + sizeof(mem_std_block_header_footer_t) * 2)
    {
        // Create a new free block from remaining space
        mem_std_free_block_t *new_block = (mem_std_free_block_t *)((char *)current + sizeof(mem_std_block_header_footer_t) + size);
        size_t remaining_size = block_size - size - sizeof(mem_std_block_header_footer_t) * 2;
        set_block_size(&(new_block->header), remaining_size);
        set_block_free(&(new_block->header));
        new_block->next = current->next;
        new_block->prev = current;
        set_block_size(&(current->header), size); // Update the current block size and footer
        current->next = new_block;
        mem_std_block_header_footer_t *new_footer = (mem_std_block_header_footer_t *)((char *)new_block + sizeof(mem_std_block_header_footer_t) + remaining_size); // footer for the new free block
        *new_footer = new_block->header;
    }

    set_block_used(&(current->header)); // Mark the block as used
    mem_std_block_header_footer_t *footer = (mem_std_block_header_footer_t *)((char *)current + sizeof(mem_std_block_header_footer_t) + get_block_size(&(current->header)));
    *footer = current->header;
    return (void *)((char *)current + sizeof(mem_std_block_header_footer_t)); // Return the memory address after the header
}

void mem_free_standard_pool(mem_pool_t *pool, void *addr)
{
    mem_std_free_block_t *block = (mem_std_free_block_t *)((char *)addr - sizeof(mem_std_block_header_footer_t)); // Get header of the free block
    set_block_free(&(block->header));                                                                             // Mark the block as free
    mem_std_block_header_footer_t *footer = (mem_std_block_header_footer_t *)((char *)block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(block->header)));
    *footer = block->header; // Update the footer to match the header

    // Next block
    mem_std_free_block_t *next_block = (mem_std_free_block_t *)((char *)footer + sizeof(mem_std_block_header_footer_t));
    if ((char *)next_block < (char *)pool->end_addr && is_block_free(&(next_block->header)))
    {
        size_t new_size = get_block_size(&(block->header)) + sizeof(mem_std_block_header_footer_t) * 2 + get_block_size(&(next_block->header)); // Merge this block with the next free block
        set_block_size(&(block->header), new_size);
        set_block_free(&(block->header));
        footer = (mem_std_block_header_footer_t *)((char *)block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(block->header))); // Update footer for new size
        *footer = block->header;

        // Remove next block from the free list
        if (next_block->next)
        {
            next_block->next->prev = block;
        }
        block->next = next_block->next;
    }

    // Previous block
    mem_std_block_header_footer_t *prev_footer = (mem_std_block_header_footer_t *)((char *)block - sizeof(mem_std_block_header_footer_t));
    if ((char *)prev_footer > (char *)pool->start_addr && is_block_free(prev_footer))
    {
        mem_std_free_block_t *prev_block = (mem_std_free_block_t *)((char *)prev_footer - get_block_size(prev_footer) - sizeof(mem_std_block_header_footer_t));
        size_t new_size = get_block_size(&(prev_block->header)) + sizeof(mem_std_block_header_footer_t) * 2 + get_block_size(&(block->header));
        set_block_size(&(prev_block->header), new_size);
        set_block_free(&(prev_block->header));
        footer = (mem_std_block_header_footer_t *)((char *)prev_block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(prev_block->header)));
        *footer = prev_block->header;
        if (block->next)
        {
            block->next->prev = prev_block;
        }
        prev_block->next = block->next;

        block = prev_block;
    }

    // Inserting block in the free list
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
        pool->first_free = block; // new head of the free list
    }
}

size_t mem_get_allocated_block_size_standard_pool(mem_pool_t *pool, void *addr)
{
    mem_std_allocated_block_t *block = (mem_std_allocated_block_t *)((char *)addr - sizeof(mem_std_block_header_footer_t));
    return get_block_size(&(block->header));
}
