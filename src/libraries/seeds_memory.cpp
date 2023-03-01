#if SEEDS_INTERNAL
#define INTERNAL_MEMORY_PARAM char *Filename, int LineNumber,
#define INTERNAL_MEMORY_PASS  __FILE__, __LINE__,
#else
#define INTERNAL_MEMORY_PARAM
#define INTERNAL_MEMORY_PASS
#endif

#define ZeroStruct(Base, Type)       ZeroSize_(INTERNAL_MEMORY_PASS (void *)Base, sizeof(Type))
#define ZeroArray(Base, Count, Type) ZeroSize_(INTERNAL_MEMORY_PASS (void *)Base, Count * sizeof(Type))
internal void
ZeroSize_(INTERNAL_MEMORY_PARAM
          void *Base, memory_index Size) {
    
#if defined(SEEDS_SIMD_H) && 1
    __m128i *Ptr128 = (__m128i *)Base;
    __m128i  Zero   =  _mm_setzero_si128();
    while (Size - 16 < Size) { Size -= 16; _mm_store_si128(Ptr128++, Zero);}
    
    u64 *Ptr64 = (u64 *)Ptr128;
    if (Size - 8 < Size)     { Size -= 8; *Ptr64++ = 0; }
    
    u32 *Ptr32 = (u32 *)Ptr64;
    if (Size - 4 < Size)     { Size -= 4; *Ptr32++ = 0; }
    
    u16 *Ptr16 = (u16 *)Ptr32;
    if (Size - 2 < Size)     { Size -= 2; *Ptr16++ = 0; }
    
    u8 *Ptr8 = (u8 *)Ptr16;
    if (Size - 1 < Size)     { Size -= 1; *Ptr8++ = 0; }
#else
    u8 *Ptr = (u8 *)Base;
    while(Size--) *Ptr++ = 0;
#endif
    
}

internal void
ClearBufferToValue(s32 *Buffer, int Width, int Height, s32 Value) {
    
    int Index = 0;
    
#ifdef SEEDS_SIMD_H
    __m128i Wide = _mm_set1_epi32(Value);
    while (Index < Width * Height - 3) {
        _mm_store_si128((__m128i *)(Buffer + Index), Wide);
        Index += 4;
    }
#endif
    
    for (; Index < Width * Height; Index++) Buffer[Index] = Value;
}

internal void
ClearBufferToValue(u32 *Buffer, int Width, int Height, u32 Value) {
    
    int Index = 0;
    
#ifdef SEEDS_SIMD_H
    __m128i Wide = _mm_set1_epi32(Value);
    while (Index < Width * Height - 3) {
        _mm_store_si128((__m128i *)(Buffer + Index), Wide);
        Index += 4;
    }
#endif
    
    for (; Index < Width * Height; Index++) Buffer[Index] = Value;
}

internal void
ClearBufferToValue(f32 *Buffer, int Width, int Height, f32 Value) {
    
    int Index = 0;
    
#ifdef SEEDS_SIMD_H
    __m128 Wide = _mm_set1_ps(Value);
    while (Index < Width * Height - 3) {
        _mm_store_ps(Buffer + Index, Wide);
        Index += 4;
    }
#endif
    
    for (; Index < Width * Height; Index++) Buffer[Index] = Value;
}




internal void
InitialiseArena(memory_arena *Arena, u8 *Base, memory_index Size) {
    
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, Type)       (Type *)PushSize_(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type *)PushSize_(Arena, sizeof(Type) * Count)
// TODO(canta): PushAndZeroArray
internal void *
PushSize_(memory_arena *Arena, memory_index Size) {
    
    Assert(Arena->Used + Size <= Arena->Size);
    
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return Result;
}

#define PopStruct(Arena, Type)       (Type *)PopSize_(INTERNAL_MEMORY_PASS Arena, sizeof(Type))
#define PopArray(Arena, Count, Type) (Type *)PopSize_(INTERNAL_MEMORY_PASS Arena, sizeof(Type) * Count) 
#define PopAndZeroArray(Arena, Count, Type)  ZeroArray(PopArray(Arena, Count, Type), Count, Type)
internal void *
PopSize_(INTERNAL_MEMORY_PARAM
         memory_arena *Arena, memory_index Size) {
    
    Assert(Arena->Used - Size >= 0);
    
    void *Result;
    
    Arena->Used -= Size;
    Result = Arena->Base + Arena->Used;
    
    return Result;
}

// TODO(canta): Go through the code and replace Pop with PopAndZero where necessary/possible. There are a lot of dangerous assumptions baked in that are going unchallenged because only RenderEntities() regularly deals with arenas.





internal void
InitialiseScratch(memory_scratch *Scratch, u8 *Base, memory_index Size) {
    
    Scratch->Base = Base;
    Scratch->Size = Size;
    Scratch->Used = 0;
    ZeroArray(Base, Size, u8);
}

internal void
ResetScratch(memory_scratch *Scratch) {
    
    ZeroArray(Scratch->Base, Scratch->Used, u8);
    Scratch->Used = 0;
}

internal void
FreeScratch(memory_scratch *Scratch) {
    
    Scratch->Base = 0;
    Scratch->Size = 0;
    Scratch->Used = 0;
}

internal void *
PushSize_(memory_scratch *Scratch, memory_index Size) {
    
    Assert(Scratch->Used + Size <= Scratch->Size);
    
    void *Result = Scratch->Base + Scratch->Used;
    Scratch->Used += Size;
    
    return Result;
}





internal heap_node *
AcquireHeapNode(u8 *Base, memory_index Size, heap_node *Next = 0) {
    
    Assert(Size >= sizeof(heap_node));
    
    heap_node *Result = (heap_node *)Base;
    
    Result->Size     = Size;
    Result->Next     = Next;
    
    return Result;
}

internal void
InitialiseHeap(memory_heap *Heap, u8 *Base, memory_index Size) {
    
    Heap->Base = Base;
    Heap->Size = Size;
    ZeroArray(Heap->Base, Heap->Size, u8);
    
    Heap->Head = AcquireHeapNode(Base, Size);
}

internal void *
HeapAlloc(memory_heap *Heap, memory_index Size) {
    
    heap_node *Prev = 0;
    heap_node *Node = Heap->Head;
    heap_node *Brev = 0;
    heap_node *Best = 0;
    
    if (Size) {
        if (Size < sizeof(heap_node)) Size = sizeof(heap_node);
        
        // Finding suitable node
        do {
            // TODO(canta): Do something cleverer here?
            
            // If the node is the tightest possbile fit
            if (Size == Node->Size) {
                Brev = Prev;
                Best = Node;
                break;
            }
            // If the node is suitable and more tightly fitted than the current best
            else if (Best) {
                if (Size < Node->Size && Node->Size < Best->Size) {
                    Brev = Prev;
                    Best = Node;
                }
            }
            // If the node is the first fit
            else if (Size < Node->Size) {
                Brev = Prev;
                Best = Node;
            }
            
            Prev = Node;
            Node = Node->Next;
        } while (Node);
        
        if (Best) {
            memory_index Remainder = Best->Size - Size;
            
            // Create a new node to contain whatever cruft is left over.
            // TODO(canta): Do we take the L(eak) here? I don't see what else can be done without storing the nodes separately. Worth doing that to avoid fragmentation?
            if (Remainder >= sizeof(heap_node)) {
                // @Incomplete(canta): Try and chunk Remainder into Brev or Best->Next.
                heap_node *New = AcquireHeapNode((u8 *)Best + Size, Remainder, Best->Next);
                if (Best == Heap->Head) Heap->Head = New;
                else                    Brev->Next = New;
            }
            else {
                if (Best == Heap->Head) Heap->Head = Best->Next;
                else                    Brev->Next = Best->Next;
            }
            
            Heap->Used += Size;
            ZeroStruct(Best, heap_node);
        }
        else {
            // TODO(canta): This is temp, do a diagnostic in proper logging system
            Assert(!"Suitable memory block not found");
        }
    }
    
    return (void *)Best;
}

internal void
HeapFree(memory_heap *Heap, void *Block, memory_index Size) {
    
    if (Size) {
        u8 *BlockP = (u8 *)Block;
        
        // TODO(canta): Is this going to cause problems? Think about it nephew....
        if (Size < sizeof(heap_node)) Size = sizeof(heap_node);
        ZeroArray(BlockP, Size, u8);
        
        if (Heap->Head && (u8 *)Heap->Head < BlockP) {
            // Navigating to the nodes which sandwich BlockP
            heap_node *Node = Heap->Head;
            heap_node *Next = 0;
            while (Node->Next && (u8 *)Node->Next < BlockP) Node = Node->Next;
            Next = Node->Next;
            
            // Squash BlockP and Next into Node
            if ((BlockP - Node->Size) == (u8 *)Node && (BlockP + Size) == (u8 *)Next) {
                Node->Size += Size + Next->Size;
                Node->Next = Next->Next;
                ZeroStruct(Next, heap_node);
            }
            
            // Squash BlockP into Node
            else if ((BlockP - Node->Size) == (u8 *)Node && (BlockP + Size) != (u8 *)Next) {
                Node->Size += Size;
            }
            
            // Squash Next into BlockP
            else if ((BlockP - Node->Size) != (u8 *)Node && (BlockP + Size) == (u8 *)Next) {
                Node->Next = AcquireHeapNode(BlockP, Size + Next->Size, Next->Next);
                ZeroStruct(Next, heap_node);
            }
            
            // No squashing
            else {
                Node->Next = AcquireHeapNode(BlockP, Size, Next);
            }
        }
        else {
            Heap->Head = AcquireHeapNode(BlockP, Size, Heap->Head);
        }
        
        Heap->Used -= Size;
    }
}

