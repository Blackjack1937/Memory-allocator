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
    p->start_addr = address;
    p->end_addr = (void *)((char *)address + size);
    p->pool_size = size;
    p->min_req_size = min_request_size;
    p->max_req_size = max_request_size;
    p->first_free = address;

    // First free block
    block_header_t *first_block = (block_header_t *)address;
    first_block->size = size - sizeof(block_header_t) - sizeof(block_footer_t);
    first_block->is_free = 1;
    first_block->next = NULL;
    first_block->prev = NULL;

    // Ajouter le footer identique au header
    block_footer_t *first_footer = (block_footer_t *)((char *)address + size - sizeof(block_footer_t));
    first_footer->size = first_block->size;
    first_footer->is_free = 1;

    printf("Standard pool initialized with a block of size %zu bytes\n", first_block->size);
}

void *mem_alloc_standard_pool(mem_pool_t *pool, size_t size)
{
    /* TO BE IMPLEMENTED */
    printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
    return NULL;
}

void mem_free_standard_pool(mem_pool_t *pool, void *addr)
{
    /* TO BE IMPLEMENTED */
    printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}

size_t mem_get_allocated_block_size_standard_pool(mem_pool_t *pool, void *addr)
{
    /* TO BE IMPLEMENTED */
    printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
    return 0;
}
