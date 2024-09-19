#include <stdio.h>
#include <assert.h>
#include <string.h>


#include "./mem_alloc_standard_pool.h"
#include "./mem_alloc_sim.h"
#include "./sim_alloc.h"

simulator_state_t *sim_state;


void memory_init_sim(size_t pool_0_size, size_t pool_1_size, size_t pool_2_size, size_t pool_3_size, policy_t policy, unsigned alignment)
{

    sim_state = sim_init(pool_0_size, pool_1_size, pool_2_size, pool_3_size, policy, alignment, sizeof(mem_std_free_block_t), sizeof(mem_std_block_header_footer_t), sizeof(mem_std_allocated_block_t));
}


    
void *memory_alloc(size_t size)
{
    return sim_alloc(sim_state, size);
}

void memory_free(void *p)
{
    sim_free(sim_state, p);
}



void print_info(void)
{
}

void print_alloc_info(void *addr, int size)
{
}

void print_free_info(void *addr)
{
}

void print_free_blocks(void)
{
}

char *heap_base(void)
{
    return NULL;
}

void print_mem_state(void)
{
}

void print_alloc_error(int size)
{
    fprintf(stderr, "ALLOC error : can't allocate %d bytes\n", size);
}
