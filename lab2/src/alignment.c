#include <stdlib.h>
#include <stdio.h>

#include "mem_alloc_standard_pool.h"


/*
/!\ constant to define in both mem_alloc_standard_pool and mem_alloc_fast_pool 
to have the nececary allignment
*/ 
const long int alignment=64;


void* align_init_fast(void* adr){  
    if (adr%alignment){
    return (void*)((char*)adr+alignment-((long int)adr%alignment));
    }else{
    return adr
    }

}

void* align_init_standard(void* adr){   
    if ((adr+sizeof(mem_std_block_header_footer_t))%alignment){
        long int modif= (((long int)adr+sizeof(mem_std_block_header_footer_t))%alignment)-sizeof(mem_std_block_header_footer_t);
        while(modif<0){
            modif+=alignment;
        }
        return (void*)((char*)adr+modif);
    }else{
    return adr
    }
}

size_t size_block_aligment(size_t s){
    if ((s+sizeof(mem_std_block_header_footer_t)*2)%alignment){
        long int modif= (((long int)s+sizeof(mem_std_block_header_footer_t)*2)%alignment)-sizeof(mem_std_block_header_footer_t)*2;
        while(modif<0){
            modif+=alignment;
        }
    }
    return s+modif;
}