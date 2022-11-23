
#include <stdlib.h>
#include<set>
#include<queue>
#include<iostream>
#include<cstring>
#include"ChunkAlloc.hpp"

template<
    class T,          // This is the type we are allocating.  You can assume this is less than or equal to 4kB
    size_t ALIGNMENT  // The alignment at which we much allocate the objects.  You can assume this is less than or equal to 4kB
    >
class AlignedAllocator {
    std::set<T*> chunks; // We store everything we allocated so we can clean up in the destructor.
    std::queue<T*> retired;
public:
    typedef T ItemType; // This will make T available as AlignedAllocator::ItemType
    static const size_t Alignment = ALIGNMENT;  // Likewise, we can access the alignment as AlignedAllocator::Alignment
    size_t curPos;
    size_t block;
    uint8_t count;
    uint capacity;
    void* p;
    void* thisChunk;
    
    AlignedAllocator() {
        block = Alignment;
        if (sizeof(T) > Alignment) {
            block = Alignment * ((sizeof(T)/Alignment)+1);
        }
        capacity = 128*1024/block;
        capacity -= 8;
        curPos = 0;
        count = 0;
    }

    // The __attribute__ ((noinline)) is here so you can find the assembly for alloc() and free().  You're welcome to remove it.
    T * __attribute__((noinline))  alloc() {
        
        
        if (retired.empty()) {
            if (count==0 || count>=capacity) {
                p = alloc_chunk();
                chunks.insert(reinterpret_cast<T*>(p));
                uint8_t * t = reinterpret_cast<uint8_t*>(p);
                for (uint j = 0; j < 128*1024; j++) {
                    t[j] = 0; // and set to zero
                }
                
                // align p
                uintptr_t n = reinterpret_cast<uintptr_t>(p);
                uintptr_t offset = reinterpret_cast<uintptr_t>(Alignment) - n % reinterpret_cast<uintptr_t>(Alignment);
                p = reinterpret_cast<T*>(n + offset);
                
                
                count = 0;
                curPos = 0;
            }
            thisChunk = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + reinterpret_cast<uintptr_t>(curPos));
            curPos += block;
            count++;
        } else {
            thisChunk = reinterpret_cast<void*>(retired.front());
            retired.pop();
        }   

        T* c = reinterpret_cast<T*>(thisChunk);
        new (c) T;
        return c;
    }

    void __attribute__((noinline))  free(T * p) {
        //std::free(reinterpret_cast<void*>(p)); // Return the memory
        //chunks.erase(p); // note that it's no longer allocated.
        
        //uint8_t * t = reinterpret_cast<uint8_t*>(p);
        //for (uint j = 0; j < sizeof(T); j++) {
        //    t[j] = 0; // and set to zero
        //}
        retired.push(p);
    }

    __attribute__((noinline)) ~AlignedAllocator() {
        for(auto & p: chunks) {
            free_chunk(reinterpret_cast<void*>(p)); // Return everything that still allocated.
        }
    }
};

template<class T, size_t ALIGNMENT>
const size_t AlignedAllocator<T, ALIGNMENT>::Alignment;
// Cfiddle-signature=6c6c3c7c0f1235e98a78f5b7738f6b0c
