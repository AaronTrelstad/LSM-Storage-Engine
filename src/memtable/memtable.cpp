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

    size_t MemTable::MemoryUsage() const
    {
        return memory_usage_;
    }

    MemTable::Iterator MemTable::NewIterator() const
    {
        return Iterator(nullptr, this);
    }

    MemTable::Iterator::Iterator(SkipList::Node *start, const MemTable *memtable) : current_(start), memtable_(memtable) {}

    void MemTable::Iterator::SeekToFirst()
    {
        current_ = memtable_->skiplist_.Head()->next[0].load(std::memory_order_acquire);
    }

    bool MemTable::Iterator::Valid() const
    {
        return current_ != nullptr;
    }

    void MemTable::Iterator::Next()
    {
        current_ = current_->next[0].load(std::memory_order_acquire);
    }

    std::string_view MemTable::Iterator::key() const
    {
        return current_->key;
    }
    std::string_view MemTable::Iterator::value() const
    {
        return current_->value;
    }
}
