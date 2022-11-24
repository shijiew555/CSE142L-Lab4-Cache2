

#include <stdlib.h>
#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <cstring>
#include "ChunkAlloc.hpp"

template<
    class T,          // This is the type we are allocating.  You can assume this is less than or equal to 4kB
    size_t ALIGNMENT  // The alignment at which we much allocate the objects.  You can assume this is less than or equal to 4kB
    >
class AlignedAllocator {
    std::vector<void*> chunks; // We store everything we allocated so we can clean up in the destructor.
    std::queue<T*> retired;
public:
    typedef T ItemType; // This will make T available as AlignedAllocator::ItemType
    static const size_t Alignment = ALIGNMENT;  // Likewise, we can access the alignment as AlignedAllocator::Alignment
    size_t curPos;
    size_t block;
    size_t count;
    size_t capacity;
    void* p;
    
    AlignedAllocator() {
        block = Alignment;
        if (sizeof(T) > Alignment) {
            block = Alignment * ((sizeof(T)/Alignment)+1);
        }
        if (Alignment % sizeof(T) == 0) {
            block = Alignment;
        }

        capacity = 128*1024/block;
        capacity--;
        curPos = 0;
        count = 0;
    }

    // The __attribute__ ((noinline)) is here so you can find the assembly for alloc() and free().  You're welcome to remove it.
    T * alloc() {
        if (retired.empty()) {
            if (count==0 || count>=capacity) {
                p = alloc_chunk();
                chunks.push_back(p);
                uint64_t * t = reinterpret_cast<uint64_t*>(p);
                for (uint j = 0; j < 16*1024; j++) {
                    t[j] = 0;
                }
                
                // align p
                uintptr_t n = reinterpret_cast<uintptr_t>(p);
                uintptr_t offset = reinterpret_cast<uintptr_t>(Alignment) - n % reinterpret_cast<uintptr_t>(Alignment);
                p = reinterpret_cast<void*>(n + offset);
                
                count = 0;
                curPos = 0;
            }
            T* c = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(p) + reinterpret_cast<uintptr_t>(curPos));
            new (c) T;
            curPos += block;
            count++;
            return c;
            
        }
        T* b = retired.front();
        retired.pop();
        return b;
        

        //return nullptr;
    }
    
    void free(T * p) {
        retired.push(p);
    }

    ~AlignedAllocator() {
        for (auto & p: chunks) {
            free_chunk(p);
        }
    }
};

template<class T, size_t ALIGNMENT>
const size_t AlignedAllocator<T, ALIGNMENT>::Alignment;

// Cfiddle-signature=7f334a9b52a9e7a1de32ba8b6369a17b
