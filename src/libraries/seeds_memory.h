#ifndef SEEDS_MEMORY_H

struct memory_arena {
    
    u8          *Base;
    memory_index Size;
    memory_index Used;
    
};

struct memory_scratch {
    
    u8           *Base;
    memory_index  Size;
    memory_index  Used;
};

struct heap_node {
    
    memory_index  Size;
    heap_node    *Next;
};

struct memory_heap {
    
    u8           *Base;
    memory_index  Size;
    memory_index  Used;
    heap_node    *Head;
};

#define SEEDS_MEMORY_H
#endif

