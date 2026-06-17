#pragma once
#include <string>
#include <string_view>
#include <cstdint>

namespace lsm
{

    class SSTableReader
    {
    public:
        explicit SSTableReader(const std::string &path);
        ~SSTableReader();

        bool Get(std::string_view key, std::string *value) const;

    private:
        std::string path_;
        int fd_;

        std::string data_block_;
        uint64_t data_block_size_;
        uint64_t filter_block_size_;
    };

}
