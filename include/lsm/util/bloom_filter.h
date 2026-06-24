#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace lsm
{
    class BloomFilter
    {
    public:
        BloomFilter(size_t num_keys, double false_positive_rate);

        void Add(std::string_view key);
        bool MayContain(std::string_view key) const;

        std::string Serialize() const;
        static BloomFilter Deserialize(std::string_view data);

    private:
        BloomFilter(size_t num_bits, int num_hashes, std::vector<bool> bits);

        uint64_t Hash1(std::string_view key) const;
        uint64_t Hash2(std::string_view key) const;

        std::vector<bool> bits_;
        size_t num_bits_;
        int num_hashes_;
    };

} 
