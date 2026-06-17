#pragma once
#include <string>
#include <string_view>
#include <functional>
#include <cstdint>

namespace lsm
{
    class WAL
    {
    public:
        explicit WAL(const std::string &path);
        ~WAL();

        void Append(uint64_t sequence, std::string_view key, std::string_view value);
        void Sync();
        void Delete();

        using RecoverCallback = std::function<void(uint64_t sequence, std::string_view key, std::string_view value)>;
        bool Recover(RecoverCallback cb);

    private:
        uint32_t ComputeChecksum(const std::string& data);

        std::string path_;
        int fd_;
    };
}
