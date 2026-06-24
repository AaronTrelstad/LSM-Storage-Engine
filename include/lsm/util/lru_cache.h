#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <cstddef>
#include <string_view>

namespace lsm
{
    class LRUCache
    {
    public:
        explicit LRUCache(size_t capacity);

        bool Get(std::string_view key, std::string *value);
        void Put(std::string_view key, std::string value);
        void Erase(std::string_view key);

        size_t Size() const;

    private:
        struct Entry
        {
            std::string key;
            std::string value;
        };

        size_t capacity_;
        std::list<Entry> list_; 
        std::unordered_map<std::string, std::list<Entry>::iterator> map_;
    };

}
