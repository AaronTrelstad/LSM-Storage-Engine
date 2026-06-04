#include "lsm/util/arena.h"

namespace lsm
{
    Arena::Arena() = default;

    Arena::~Arena()
    {
        for (char *block : blocks_)
        {
            delete[] block;
        }
    }

    void *Arena::AllocateFallback(size_t size)
    {
        char *block = new char[4096];
        blocks_.push_back(block);
        bytes_allocated_ += 4096;
        current_ptr_ = block + size;
        remaining_ = 4096 - size;
        return block;
    }
}
