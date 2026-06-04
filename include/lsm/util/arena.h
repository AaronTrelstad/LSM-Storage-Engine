#pragma once
#include <cstddef>
#include <vector>

namespace lsm
{
    class Arena
    {
    public:
        Arena();
        ~Arena();

        inline void *Allocate(size_t size)
        {
            if (size <= remaining_)
            {
                void *ptr = current_ptr_;
                current_ptr_ += size;
                remaining_ -= size;
                return ptr;
            }

            return AllocateFallback(size);
        }

        size_t MemoryUsage() const { return bytes_allocated_; }

    private:
        void *AllocateFallback(size_t size);

        char *current_ptr_ = nullptr;
        size_t remaining_ = 0;
        size_t bytes_allocated_ = 0;
        std::vector<char *> blocks_;
    };
}
