#ifndef   	_SIM_ALLOC_H_
#define   	_SIM_ALLOC_H_

#include <stdlib.h>
#include <stdint.h>


#define MAX_BLOCK 1024

typedef enum policy{
    BF,
    WF,
    FF,
    NF
} policy_t;

/* allocated blocks*/
typedef struct simulator_block{
    int index;
    int size;
    int next;
}simulator_block_t;


/* data structure to deal with the fast pools */
typedef struct simulator_fast_pool{
    int id;
    int block_size;
    int nb_blocks;
    int* blocks;
    int first;
}simulator_fast_pool_t;

typedef union fast_pool_addr{
    struct FPA{
        uint64_t index : 48;
        uint64_t poolid : 16;
    } fpa;
    void* raw_addr;
}fast_pool_addr_t;


typedef struct simulator_state{
    simulator_fast_pool_t fpool[3];
    char *memory;
    size_t memory_size;
    unsigned mem_align_value;
    policy_t alloc_policy;
    int start_searching_index; /* use for next fit */
    int nb_blocks;
    int first_block;
    simulator_block_t stack[MAX_BLOCK];
    size_t free_header_size;
    size_t free_footer_size;
    size_t alloc_header_size;
} simulator_state_t;
    


/* Allocator sim functions*/
simulator_state_t* sim_init(size_t pool_0_size, size_t pool_1_size, size_t pool_2_size, size_t pool_3_size, policy_t policy, unsigned alignement, size_t free_header_size, size_t free_footer_size, size_t alloc_header_size);
void *sim_alloc(simulator_state_t *state, size_t size);
void sim_free(simulator_state_t *state, void *p);
size_t sim_get_allocated_block_size(simulator_state_t *state, void *addr);

#endif 	    /* !_SIM_ALLOC_H_ */
