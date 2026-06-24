#include "lsm/util/bloom_filter.h"
#include "lsm/util/coding.h"

#include <cmath>
#include <functional>

namespace lsm
{
    BloomFilter::BloomFilter(size_t num_keys, double false_positive_rate)
    {
        double ln2 = std::log(2.0);
        double m = -(static_cast<double>(num_keys) * std::log(false_positive_rate)) / (ln2 * ln2);

        num_bits_ = static_cast<size_t>(std::ceil(m));
        if (num_bits_ < 1)
            num_bits_ = 1;

        double k = (static_cast<double>(num_bits_) / static_cast<double>(num_keys)) * ln2;
        num_hashes_ = static_cast<int>(std::round(k));
        if (num_hashes_ < 1)
            num_hashes_ = 1;
        if (num_hashes_ > 30)
            num_hashes_ = 30;

        bits_.assign(num_bits_, false);
    }

    BloomFilter::BloomFilter(size_t num_bits, int num_hashes, std::vector<bool> bits)
        : bits_(std::move(bits)), num_bits_(num_bits), num_hashes_(num_hashes)
    {
    }

    uint64_t BloomFilter::Hash1(std::string_view key) const
    {
        return std::hash<std::string_view>{}(key);
    }

    uint64_t BloomFilter::Hash2(std::string_view key) const
    {
        std::string salted = std::string(key) + "_salt";
        return std::hash<std::string>{}(salted);
    }

    void BloomFilter::Add(std::string_view key)
    {
        uint64_t h1 = Hash1(key);
        uint64_t h2 = Hash2(key);

        for (int i = 0; i < num_hashes_; i++)
        {
            uint64_t combined = h1 + static_cast<uint64_t>(i) * h2;
            size_t bit_pos = combined % num_bits_;
            bits_[bit_pos] = true;
        }
    }

    bool BloomFilter::MayContain(std::string_view key) const
    {
        uint64_t h1 = Hash1(key);
        uint64_t h2 = Hash2(key);

        for (int i = 0; i < num_hashes_; i++)
        {
            uint64_t combined = h1 + static_cast<uint64_t>(i) * h2;
            size_t bit_pos = combined % num_bits_;

            if (!bits_[bit_pos])
            {
                return false;
            }
        }
        return true;
    }

    std::string BloomFilter::Serialize() const
    {
        std::string out;

        char header[12];
        EncodeFixed64(header, num_bits_);
        EncodeFixed32(header + 8, static_cast<uint32_t>(num_hashes_));
        out.append(header, sizeof(header));

        size_t num_bytes = (num_bits_ + 7) / 8;
        std::string packed(num_bytes, '\0');

        for (size_t i = 0; i < num_bits_; i++)
        {
            if (bits_[i])
            {
                packed[i / 8] |= (1 << (i % 8));
            }
        }

        out.append(packed);
        return out;
    }

    BloomFilter BloomFilter::Deserialize(std::string_view data)
    {
        uint64_t num_bits = DecodeFixed64(data.data());
        int num_hashes = static_cast<int>(DecodeFixed32(data.data() + 8));

        std::vector<bool> bits(num_bits, false);
        const char *packed = data.data() + 12;

        for (uint64_t i = 0; i < num_bits; i++)
        {
            bool bit_set = (packed[i / 8] >> (i % 8)) & 1;
            bits[i] = bit_set;
        }

        return BloomFilter(num_bits, num_hashes, std::move(bits));
    }
}
