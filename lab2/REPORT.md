# OS - Lab 2 : Memory allocator design

# Report

### Participants : RIDAOUI Hatim & MORONI Louka - G2

## Platform description :

## Implemented features :

## Main design choices :

## Questions raised in the assignement :

### - Step 1 :

    1. For standard blocks, the metadata used to describe the memory blocks inside the memory consists of a Header/Footer combination for each block. It is unnecessary to do the same for fast blocks, as they all have a fixed size, and describe each other using the linked-list concept.

    2. The minimum size of a free block in any given pool depends on the pointer size that points to the next free block (linked-list concept). Same goes for an allocated block, that corresponds to the minimum size for a free-block pointer, in case we free that block.

    3. No, we do not need to keep a list for the allocated blocks: the memory_free function takes as parameter the pointer to the freed block, that we can use to reposition it in the free list.

    4. In a free block, we should include a pointer to the next free block. In an allocated block, we do not include any metadata.

    5. Using a LI-FO policy, the freed block should be inserted at the beginning the list, adressed to the "next pointer" of the previous free block.

    - Why a LIFO policy for free block allocation is interesting in the context of fast pools ?

        We use a LIFO policy in order to assure a simple and efficient organization for the fast pools, in which all the informations relating to blocks is contained in the list itself, as the address of the list always points to the next block to be allocated (first free block).

## Test scenarios :

    gdb ./bin./mem_shell
    run <tests/alloc1.in

## Conclusion and feedback :
