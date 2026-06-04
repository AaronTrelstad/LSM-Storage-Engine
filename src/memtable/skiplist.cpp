#include "lsm/memtable/skiplist.h"

namespace lsm
{
    SkipList::SkipList(int max_levels)
        : max_level_(max_levels)
        , current_level_(1)
        , gen_(std::random_device{}())
        , distrib_(1, max_levels)
        , head_(new Node("", "", max_levels))
    {}

    bool SkipList::Get(std::string_view key, std::string *value) const
    {
        Node *current = head_;

        for (int i = current_level_ - 1; i >= 0; i--)
        {
            while (true)
            {
                Node *next = current->next[i].load(std::memory_order_acquire);

                if (next == nullptr)
                    break;

                if (key == next->key)
                {
                    *value = next->value;
                    return true;
                }
                else if (key > next->key)
                {
                    current = next;
                }
                else
                {
                    break;
                }
            }
        }
        return false;
    }

    bool SkipList::Put(std::string_view key, std::string_view value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        std::vector<Node *> updates(max_level_, nullptr);
        Node *current = head_;

        for (int i = current_level_ - 1; i >= 0; i--)
        {
            while (true)
            {
                Node *next = current->next[i].load(std::memory_order_relaxed);

                if (next == nullptr)
                    break;

                if (key == next->key)
                {
                    next->value = value;
                    return false;
                }
                else if (key > next->key)
                {
                    current = next;
                }
                else
                {
                    break;
                }
            }
            updates[i] = current;
        }

        int level = RandomLevel();

        if (level > current_level_)
        {
            for (int i = current_level_; i < level; i++)
                updates[i] = head_;
            current_level_ = level;
        }

        Node *new_node = new Node(key, value, level);

        for (int i = 0; i < level; i++)
        {
            new_node->next[i].store(
                updates[i]->next[i].load(std::memory_order_relaxed),
                std::memory_order_relaxed);
            updates[i]->next[i].store(new_node, std::memory_order_release);
        }

        return true;
    }

    int SkipList::RandomLevel()
    {
        return distrib_(gen_);
    }

}
