#include <assert.h>
#include <stdio.h>

#include "mem_alloc_fast_pool.h"
#include "my_mmap.h"
#include "mem_alloc.h"

void init_fast_pool(mem_pool_t *p, size_t size, size_t min_request_size, size_t max_request_size)
{
    /* TO BE IMPLEMENTED */
    printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}

void *mem_alloc_fast_pool(mem_pool_t *pool, size_t size)
{
    /* TO BE IMPLEMENTED */
    printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
    return NULL;
}

void mem_free_fast_pool(mem_pool_t *pool, void *b)
{
    /* TO BE IMPLEMENTED */
    printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}

size_t mem_get_allocated_block_size_fast_pool(mem_pool_t *pool, void *addr)
{
    size_t res;
    res = pool->max_req_size;
    return res;
}
