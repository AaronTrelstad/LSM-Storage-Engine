#pragma once
#include <string>
#include <string_view>
#include <cstddef>
#include "lsm/memtable/skiplist.h"

namespace lsm
{
    class MemTable
    {
    public:
        explicit MemTable(size_t max_size = 64 * 1024 * 1024);

        void Put(std::string_view key, std::string_view value);
        bool Get(std::string_view key, std::string *value) const;

        size_t MemoryUsage() const;
        bool CanFit(size_t entry_size) const;

        class Iterator
        {
        public:
            void SeekToFirst();
            bool Valid() const;
            void Next();
            std::string_view key() const;
            std::string_view value() const;

        private:
            friend class MemTable;
            explicit Iterator(SkipList::Node *start);
            SkipList::Node *current_;
        };

        Iterator NewIterator() const;

    private:
        friend class Iterator;

        SkipList skiplist_{12};
        size_t memory_usage_ = 0;
        const size_t max_size_;
    };

}
