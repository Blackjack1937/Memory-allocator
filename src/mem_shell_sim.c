#include <stdio.h>
#include <string.h>

#include "mem_alloc_sim.h"

#define SIZE_BUFFER 128


void help(void){
    printf("List of commands:\n");
    printf("\t h -- displays this help message\n");
    printf("\t aXX -- allocates a block of size XX\n");
    printf("\t fY -- frees the block allocated in the Y-th call to malloc\n");
    printf("\t p -- displays the memory state\n");
    printf("\t q -- exists the program\n");
}

int main(int argc, char *argv[]) {
    char buffer[SIZE_BUFFER];
    char command;
    int index;
    int size;

    char* block_pointer[1024];
    int count=1;
    policy_t pool_policy = BF;
    int align = 1;

    /* size of each pool for the simulator */
    size_t pool_size[4]={0, 0, 0, 0};

    
    if(argc > 4){
        int i=0;
        for(i = 0; i<4; i++){
            pool_size[i]=atoi(argv[i+1]);
            printf("Config: pool size for pool %d -> %zu\n",i,pool_size[i]);
        }

        if(argc > 5){
            if(!strcmp(argv[5], "WF")){
                pool_policy = WF;
            }
            else if(!strcmp(argv[5], "FF")){
                pool_policy = FF;
            }
            else if(!strcmp(argv[5], "NF")){
                pool_policy = NF;
            }
        }
        if(argc > 6){
            int val = atoi(argv[6]);
            align = (val <=64 && val > 1)? val : 1;
        }
    }
    else{
        printf("Config: running test with the default configuration\n");
    }
        
    memset(block_pointer,0,1024*sizeof(char*));
    
    memory_init_sim(pool_size[0], pool_size[1], pool_size[2], pool_size[3], pool_policy, align);

    while (1) {
        command = getchar();
        
        switch (command) {
        case 'a':
            scanf ("%d",&size);
            block_pointer[count]=memory_alloc(size);
            count++;
            break;
        case 'f': 
            scanf("%d",&index);
            memory_free(block_pointer[index]);
            break;
        case 'p':
            print_mem_state();
            break;
        case 'h':
            help();
            break;
        case 'q':
            exit(0);
        default:
            fprintf(stderr,"Command not found !\n");
        }
        /* empty stdin if some chars remain*/
        fgets(buffer,SIZE_BUFFER,stdin);
    }
    return 0;
}
