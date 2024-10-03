#include <assert.h>
#include <stdio.h>

#include "mem_alloc_fast_pool.h"
#include "mem_alloc.h"
#include "my_mmap.h"

void init_fast_pool(mem_pool_t *p, size_t size, size_t min_request_size, size_t max_request_size)
{
    size_t block_size;
    size_t nb_blocks;

    switch (p->pool_id)
    {
    case 0:
        block_size = 64; // Pool 0 block size is 64 bytes
        break;
    case 1:
        block_size = 256; // Pool 1 block size is 256 bytes
        break;
    case 2:
        block_size = 1024; // Pool 2 block size is 1024 bytes
        break;
    default:
        printf("Error: Unsupported pool id\n");
        return;
    }


    // Allocate memory for the pool using my_mmap
    void *address = my_mmap(size);
    if (address == NULL)
    {
        perror("Memory allocation failed");
        return;
    }

    /* 
    as the blocks for each fast pool has a size thats is a multiple of 2, 4, 8, 16, 32, 64
    and that the blocks are back to back, and that there is no header that could cause it to go of allignment
    we only need to offest the startign adress in a way that makes it a multiple of the desired alignement
    */
    long int alignment_offset = (long int)address%(long int)MEM_ALIGN;
    address = (void*)((char*)address+alignment_offset);
    // Calculate the number of blocks in the pool
    nb_blocks = (size-alignment_offset) / block_size;

    // Initialize the pool
    p->start_addr = address;                        // Start of the memory region
    p->end_addr = (void *)((char *)address + size); // End of the memory region
    p->pool_size = size;
    p->min_req_size = min_request_size;
    p->max_req_size = max_request_size;
    p->first_free = address; // Initialize the free list with the first block

    // Link each block in the pool to form the free list
    for (size_t i = 0; i < nb_blocks - 1; i++)
    {
        void *current_block = (void *)((char *)address + (i * block_size));
        void *next_block = (void *)((char *)current_block + block_size);

        // Store the pointer to the next free block inside the current block
        *(void **)current_block = next_block;
    }

    // The last block points to NULL
    void *last_block = (void *)((char *)address + ((nb_blocks - 1) * block_size));
    *(void **)last_block = NULL;

    printf("Fast pool initialized with %zu blocks of size %zu bytes\n", nb_blocks, block_size);
}

void *mem_alloc_fast_pool(mem_pool_t *pool, size_t size)
{
    // Check the requested size
    if (size > pool->max_req_size || size < pool->min_req_size)
    {
        printf("Error: Requested size out of bounds for this pool\n");
        return NULL;
    }

    // Check if the free list is empty
    if (pool->first_free == NULL)
    {
        return NULL; // No free blocks available in this pool
    }

    // Allocate the first block from the free list and update it
    void *allocated_block = pool->first_free;
    pool->first_free = *(void **)allocated_block;

    return allocated_block;
}

void mem_free_fast_pool(mem_pool_t *pool, void *b)
{
    *(void **)b = pool->first_free; // The current first free block is now after this one
    pool->first_free = b;           // The freed block becomes the new first free block
}

size_t mem_get_allocated_block_size_fast_pool(mem_pool_t *pool, void *addr)
{
    size_t res;
    res = pool->max_req_size;
    return res;
}
