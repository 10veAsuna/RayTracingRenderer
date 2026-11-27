#pragma once
#include <cstdint>
#include <cstdlib>
#include <vector>

class MemoryPool
{
public:
    static constexpr size_t ChunkSize = 4096;
    
    void* Allocate(size_t size)
    {
        if (mChunks.empty() || mCurrentPosition + size > ChunkSize)
        {
            uint8_t* mem = (uint8_t*)malloc(ChunkSize);
            mChunks.push_back(mem);
            mCurrentPosition = 0;
        }
        
        void* allocatedMemory = mChunks.back() + mCurrentPosition;
        mCurrentPosition += size;
        return allocatedMemory;
    }
    
    ~MemoryPool()
    {
        for (auto it : mChunks)
        {
            free(it);
        }
    }
    
    static MemoryPool& GetInstance()
    {
        static MemoryPool instance;
        return instance;
    }
private:
    std::vector<uint8_t*> mChunks;
    int mCurrentPosition = 0;
};
