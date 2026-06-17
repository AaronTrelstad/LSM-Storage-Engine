#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace lsm
{
    class SSTableBuilder
    {
    public:
        explicit SSTableBuilder(const std::string &path);
        ~SSTableBuilder();

        void Add(std::string_view key, std::string_view value);
        void Finish();

    private:
        std::string path_;
        int fd_;
        std::string data_buf_;
        std::vector<std::string> keys_;
        uint64_t data_block_size_;
    };

}
