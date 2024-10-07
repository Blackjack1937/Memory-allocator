#include <stdlib.h>
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

    /*
    handling offset to keep the pool aligned
    */
    address=align_init_standard(address);

    // Initialize the pool
    p->start_addr = address;
    p->end_addr = (void *)((char *)address + size);
    p->pool_size = size;
    p->min_req_size = min_request_size;
    p->max_req_size = max_request_size;
    p->first_free = address;

    // First free block
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

void *mem_alloc_standard_pool(mem_pool_t *pool, size_t requested_size)
{

    // this switch is used to defin which is the best block according to the chosen placement policie and assign it to current
    switch (std_pool_policy)
    {
    case FIRST_FIT:
        // Traverse the free list from the beggining to find the first fit block
        current = (mem_std_free_block_t *)pool->first_free;
        while (current != NULL)
        {
            char is_free = is_block_free(&(current->header));
            char is_sufficently_big = get_block_size(&(current->header)) >= requested_size;
            if (is_free && is_sufficently_big)
            {
                break; // Found a suitable block
            }
            current = current->next;
        }
        break;

    case BEST_FIT:

        // Traverse the free list to find the block whose size is closest to the allocated size
        current = (mem_std_free_block_t *)pool->first_free;
        mem_std_free_block_t *best = current;
        while (current != NULL)
        {
            char is_free = is_block_free(&(current->header));
            char is_sufficently_big = get_block_size(&(current->header)) >= requested_size;
            // evaluating if the current block is closer to the desired size than the precedent best (considering that it is already big enough)
            char is_best_fit = get_block_size(&(current->header)) < get_block_size(&(best->header));
            if (is_free && is_sufficently_big && is_best_fit)
            {
                best = current;
            }
            current = current->next;
        }
        current = best;
        break;

    case NEXT_FIT:

        while (current != NULL)
        {
            char is_free = is_block_free(&(current->header));
            char is_sufficently_big = get_block_size(&(current->header)) >= requested_size;
            if (is_free && is_sufficently_big)
            {
                break; // Found a suitable block
            }
            current = current->next;
            if (current == NULL)
            {
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

    // the total size we need to alocate a block of size requested_size
    size_t total_allocated_size = requested_size + sizeof(mem_std_block_header_footer_t) * 2;
    // the size that the new block would have in case of split (without header and footers)
    size_t remaining_size = get_block_size(&(current->header)) - total_allocated_size;
    // If the remaining size allows for a new free block then we split
    if (remaining_size + sizeof(mem_std_block_header_footer_t) * 2 > sizeof(mem_std_free_block_t) + sizeof(mem_std_block_header_footer_t))
    {
        // There is enough space for a new block, so split
        mem_std_free_block_t *new_free_block = (mem_std_free_block_t *)((char *)current + total_allocated_size);

        // Set the size and mark the new block as free
        set_block_size(&(new_free_block->header), remaining_size);
        set_block_free(&(new_free_block->header));

        // Update the next and prev pointers for the new block
        new_free_block->next = current->next;
        new_free_block->prev = current;
        if (current->next != NULL)
        {
            current->next->prev = new_free_block;
        }
        current->next = new_free_block;

        // Set the footer for the new free block
        mem_std_block_header_footer_t *new_footer = (mem_std_block_header_footer_t *)((char *)new_free_block + remaining_size);
        *new_footer = new_free_block->header;

        // Adjust the current block size
        set_block_size(&(current->header), requested_size);
    }

    /*
    handling offset to keep the pool aligned
    */
    set_block_size(&(current->header),size_block_aligment(requested_size));
    
    // removing the allocated block from the free list
    if (current->prev != NULL)
    {
        current->prev->next = current->next;
    }
    if (current->next != NULL)
    {
        current->next->prev = current->prev;
    }
    if (current == pool->first_free)
    {
        pool->first_free = current->next;
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
    // Get the address of the header of the block being freed by subtracting the size of the header
    mem_std_free_block_t *freed_block = (mem_std_free_block_t *)((char *)addr - sizeof(mem_std_block_header_footer_t));

    // Mark the block as free
    set_block_free(&(freed_block->header));

    // Get the block footer
    mem_std_block_header_footer_t *freed_block_footer = (mem_std_block_header_footer_t *)((char *)freed_block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(freed_block->header)));
    *freed_block_footer = freed_block->header; // Update the footer to match the header

    // Coalesce with the next block if it's free
    mem_std_free_block_t *next_block = (mem_std_free_block_t *)((char *)freed_block_footer + sizeof(mem_std_block_header_footer_t));
    if ((char *)next_block < (char *)pool->end_addr && is_block_free(&(next_block->header)))
    {
        // Merge current block with the next free block
        size_t new_size = get_block_size(&(freed_block->header)) + sizeof(mem_std_block_header_footer_t) * 2 + get_block_size(&(next_block->header));
        set_block_size(&(freed_block->header), new_size);
        set_block_free(&(freed_block->header));

        // Update footer to match the new size
        freed_block_footer = (mem_std_block_header_footer_t *)((char *)freed_block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(freed_block->header)));
        *freed_block_footer = freed_block->header;

        // Remove next block from the free list
        if (next_block->next)
        {
            next_block->next->prev = freed_block;
        }
        freed_block->next = next_block->next;
    }

    // Coalesce with the previous block if it's free
    mem_std_block_header_footer_t *prev_footer = (mem_std_block_header_footer_t *)((char *)freed_block - sizeof(mem_std_block_header_footer_t));

    mem_std_free_block_t *prev_block;
    if ((char *)freed_block > (char *)(pool->start_addr))
    {
        // Get the previous block header
        prev_block = (mem_std_free_block_t *)((char *)prev_footer - get_block_size(prev_footer) - sizeof(mem_std_block_header_footer_t));
    }
    else
    {
        prev_block = NULL;
    }

    if ((char *)prev_block >= (char *)pool->start_addr && is_block_free(prev_footer))
    {
        // Merge the previous block with the current block
        size_t new_size = get_block_size(&(prev_block->header)) + sizeof(mem_std_block_header_footer_t) * 2 + get_block_size(&(freed_block->header));
        set_block_size(&(prev_block->header), new_size);
        set_block_free(&(prev_block->header));

        // Update footer of the merged block
        freed_block_footer = (mem_std_block_header_footer_t *)((char *)prev_block + sizeof(mem_std_block_header_footer_t) + get_block_size(&(prev_block->header)));
        *freed_block_footer = prev_block->header;

        // Remove current block from the free list
        if (freed_block->next)
        {
            freed_block->next->prev = prev_block;
        }
        prev_block->next = freed_block->next;

        freed_block = prev_block; // Continue coalescing from the previous block
    }

    // Insert the freed (or coalesced) block back into the free list
    mem_std_free_block_t *will_be_next = (mem_std_free_block_t *)pool->first_free;
    mem_std_free_block_t *prev_will_be = NULL;

    // Go to through the free list to set will_be_next to the position next to block
    while (will_be_next != NULL && (char *)will_be_next < (char *)freed_block)
    {
        prev_will_be = will_be_next;
        will_be_next = will_be_next->next;
    }

    freed_block->next = will_be_next;
    freed_block->prev = prev_will_be;
    if (will_be_next != NULL)
    {
        will_be_next->prev = freed_block;
    }
    if (prev_will_be != NULL)
    {
        prev_will_be->next = freed_block;
    }
    else
    {
        pool->first_free = freed_block; // This is the new head of the free list
    }
}

size_t mem_get_allocated_block_size_standard_pool(mem_pool_t *pool, void *addr)
{
    mem_std_allocated_block_t *block = (mem_std_allocated_block_t *)((char *)addr - sizeof(mem_std_block_header_footer_t));
    return get_block_size(&(block->header));
}
