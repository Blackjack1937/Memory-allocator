#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_

#include <stdint.h>

typedef enum {FAST_POOL = 1, STANDARD_POOL = 2} pool_category_t ; 

typedef struct mem_pool {
    int pool_id;
    const char *pool_name; 
    size_t pool_size; /* initial size, in bytes (including metadata) */
    size_t min_req_size; /* min size (in bytes) managed by this pool */ 
    size_t max_req_size; /* max size (in bytes) managed by this pool */ 
    void *start_addr; /* smallest address in the heap */
    void *end_addr;   /* highest address in the heap */
    void *first_free; /* first block in the free list */
    pool_category_t pool_type;
} mem_pool_t;


#endif /* !_MEM_ALLOC_TYPES_H_ */
