
#include <stdlib.h>
#include<set>
#include<iostream>
#include<cstring>
#include"ChunkAlloc.hpp"
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

template<
    class T,          // This is the type we are allocating.  You can assume this is less than or equal to 4kB
    size_t ALIGNMENT  // The alignment at which we much allocate the objects.  You can assume this is less than or equal to 4kB
    >
class AlignedAllocator {
    std::set<T*> chunks; // We store everything we allocated so we can clean up in the destructor.
    T* curChunk;
    std::vector<T*> blockpos;
    size_t filledSize;
    std::map<void*, void*> ptrmap;
    
public:
    typedef T ItemType; // This will make T available as AlignedAllocator::ItemType
    static const size_t Alignment = ALIGNMENT;  // Likewise, we can access the alignment as AlignedAllocator::Alignment
    
    
    AlignedAllocator() {}
    
    T * __attribute__((noinline))  alloc() {
        size_t chunkSize = 128*1024-sizeof(T);
        
        
        //std::ofstream txt;
        //txt.open("record.txt", std::ios_base::app);
        /////// record
        //txt << get_allocated_chunks() << "\n";
        //txt.close();
        
        std::ofstream txt;
        txt.open("record.txt", std::ios_base::app);
        
        if (chunks.size() == 0 || (chunks.size() != 0 && filledSize >= chunkSize && blockpos.size() == 0)) {
            
            void* p = alloc_chunk();
            
            if (p == NULL) {
                std::cerr << "alloc_chunk() failed.  Exiting: " << strerror(1) << "(" << 1 << ")\n";
                exit(1);
            }
            void* tmp = p;
            uintptr_t n = reinterpret_cast<uintptr_t>(p);
            uintptr_t offset = Alignment - n % Alignment;
            p = reinterpret_cast<T*>(n + offset);
            
            filledSize = 0;
            
            // map aligned addr to original addr
            ptrmap[p] = tmp;
            uint8_t * t = reinterpret_cast<uint8_t*>(p); 
            for(uint i= 0; i < sizeof(T); i++) {
                t[i] = 0;
            }
            T* c = reinterpret_cast<T*>(p);
            /////// record
            txt << get_allocated_chunks() << "\n";
            txt << 1 << "\n";
            txt << chunkSize << "\n";
            txt << filledSize << "\n";
            txt << c << "\n";
            txt.close();
            
            curChunk = c;
            chunks.insert(c);
            new (c) T;
            return curChunk;
        } else if (chunks.size() != 0) {
            
            if (filledSize < (chunkSize - sizeof(T))) {
                filledSize += sizeof(T);
                while (filledSize % Alignment != 0) {
                    filledSize++;
                }
                
                
                
                uint8_t * t = reinterpret_cast<uint8_t*>(curChunk + filledSize); 
                for(uint i= 0; i < sizeof(T); i++) {
                    t[i] = 0;
                }
                T* b = curChunk + filledSize/4;
                
                
                
                
                /////// record
                txt << get_allocated_chunks() << "\n";
                txt << 2 << "\n";
                txt << chunkSize << "\n";
                txt << filledSize << "\n";
                txt << curChunk << "\n";
                txt << b << "\n";
                txt.close();
                
                new (b) T;
                return b;
            } else {
                if (blockpos.size() > 0) {
                    /////// record
                    txt << get_allocated_chunks() << "\n";
                    txt << 3 << "\n";
                    //txt << c << "\n";
                    txt.close();
                    T* ret = *(blockpos.begin());
                    blockpos.erase(blockpos.begin());
                    return ret;
                }
                void* p = alloc_chunk();
            
                if (p == NULL) {
                    std::cerr << "alloc_chunk() failed.  Exiting: " << strerror(1) << "(" << 1 << ")\n";
                    exit(1);
                }
                void* tmp = p;
                uintptr_t n = reinterpret_cast<uintptr_t>(p);
                uintptr_t offset = Alignment - n % Alignment;
                p = reinterpret_cast<T*>(n + offset);

                filledSize = 0;

                // map aligned addr to original addr
                ptrmap[p] = tmp;
                uint8_t * t = reinterpret_cast<uint8_t*>(p); 
                for(uint i= 0; i < sizeof(T); i++) {
                    t[i] = 0;
                }
                T* c = reinterpret_cast<T*>(p);
                curChunk = c;
                
                /////// record
                txt << get_allocated_chunks() << "\n";
                txt << 4 << "\n";
                txt << c << "\n";
                txt.close();
                
                chunks.insert(c); 
                new (c) T;
                return curChunk;
            }
        }
        
        return nullptr;
    }

    void __attribute__((noinline))  free(T * p) {
        blockpos.push_back(p);
        uint8_t * t = reinterpret_cast<uint8_t*>(p);
        for(uint i= 0; i < sizeof(T); i++) {
            t[i] = 0;
        }
    }

    __attribute__((noinline)) ~AlignedAllocator() {
        for(auto & p: chunks) {
            free_chunk(ptrmap[reinterpret_cast<void*>(p)]); 
        }
    }
};

template<class T, size_t ALIGNMENT>
const size_t AlignedAllocator<T, ALIGNMENT>::Alignment;

// Cfiddle-signature=4b64a256312e68c44f33e4264ddc2b6e