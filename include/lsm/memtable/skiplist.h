#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <atomic>
#include <mutex>

namespace lsm
{
    class SkipList
    {
    public:
        SkipList(int max_levels);
        bool Get(std::string_view key, std::string *value) const;
        bool Put(std::string_view key, std::string_view value);

        struct Node
        {
            std::string key;
            std::string value;
            std::vector<std::atomic<Node *>> next;

            Node(std::string_view k, std::string_view v, int levels)
                : key(k), value(v), next(levels)
            {
                for (auto &ptr : next)
                    ptr.store(nullptr, std::memory_order_relaxed);
            }
        };

        Node *Head() const { return head_; }

    private:
        int RandomLevel();

        std::mt19937 gen_;
        std::uniform_int_distribution<> distrib_;
        Node *head_;
        int max_level_;
        int current_level_;
        std::mutex mutex_;
    };

}
