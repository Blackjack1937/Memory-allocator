#include <stdlib.h>
#include <stdio.h>

#include "mem_alloc_standard_pool.h"


/*
/!\ constant to define in both mem_alloc_standard_pool and mem_alloc_fast_pool 
to have the nececary allignment
*/ 
const long int alignment=64;

/*
    handling offset to keep the pool aligned 
    as the block are fixed size and all multiples of 2,4,8,16,32,64
    this is the only alignment we need to do for fast pools
    */
void* align_init_fast(void* adr);

/*
return a new adress where the first block will be aligned
*/
void* align_init_standard(void* adr);

/*
adjust the size of the block in away that the next one is aligned
*/
size_t size_block_aligment(size_t s);