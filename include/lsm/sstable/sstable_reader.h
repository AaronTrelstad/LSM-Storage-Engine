#pragma once
#include <string>
#include <string_view>
#include <functional>
#include <cstdint>

namespace lsm
{
    class SSTableReader
    {
    public:
        explicit SSTableReader(const std::string &path);
        ~SSTableReader();

        bool Get(std::string_view key, std::string *value) const;
        const std::string &GetDataBlock() const { return data_block_; }
        void ForEach(std::function<void(std::string_view, std::string_view)> cb) const;
        static bool SearchBlock(std::string_view block, std::string_view key, std::string *value);

    private:
        std::string path_;
        int fd_;

        std::string data_block_;
        uint64_t data_block_size_;
        uint64_t filter_block_size_;
    };

}
