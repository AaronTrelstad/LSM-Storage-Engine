#include "lsm/memtable/memtable.h"

namespace lsm
{
    MemTable::MemTable(size_t max_size) : max_size_(max_size) {};

    bool MemTable::Get(std::string_view key, std::string *value) const
    {
        return skiplist_.Get(key, value);
    }

    void MemTable::Put(std::string_view key, std::string_view value)
    {
        bool inserted = skiplist_.Put(key, value);

        if (inserted)
        {
            memory_usage_ += key.size() + value.size();
        }
    }

    bool MemTable::CanFit(size_t entry_size) const
    {
        return memory_usage_ + entry_size <= max_size_;
    }

    size_t MemTable::MemoryUsage() const {
        return memory_usage_;
    }
}
