#include "lsm/util/lru_cache.h"

namespace lsm
{

    LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {}

    bool LRUCache::Get(std::string_view key, std::string *value)
    {
        auto it = map_.find(std::string(key));
        if (it == map_.end())
            return false;

        list_.splice(list_.begin(), list_, it->second);
        *value = it->second->value;
        return true;
    }

    void LRUCache::Put(std::string_view key, std::string value)
    {
        auto it = map_.find(std::string(key));
        if (it != map_.end())
        {
            it->second->value = std::move(value);
            list_.splice(list_.begin(), list_, it->second);
            return;
        }

        if (map_.size() >= capacity_)
        {
            map_.erase(list_.back().key);
            list_.pop_back();
        }

        std::string k(key);
        list_.push_front({k, std::move(value)});
        map_[k] = list_.begin();
    }

    void LRUCache::Erase(std::string_view key)
    {
        auto it = map_.find(std::string(key));
        if (it == map_.end())
            return;
        list_.erase(it->second);
        map_.erase(it);
    }

    size_t LRUCache::Size() const
    {
        return map_.size();
    }

}
