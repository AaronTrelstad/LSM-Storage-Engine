#include "lsm/wal/wal.h"
#include "lsm/util/coding.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

namespace lsm
{
    WAL::WAL(const std::string &path) : path_(path), fd_(-1)
    {
        fd_ = open(path.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd_ < 0)
        {
            throw std::runtime_error("WAL: failed to open file: " + path);
        }
    }

    WAL::~WAL()
    {
        if (fd_ >= 0)
        {
            close(fd_);
        }
    }

    void WAL::Append(uint64_t sequence, std::string_view key, std::string_view value)
    {
        std::string record;

        char sequence_buffer[8];
        EncodeFixed64(sequence_buffer, sequence);
        record.append(sequence_buffer, 8);

        PutLengthPrefixedSlice(&record, key);
        PutLengthPrefixedSlice(&record, value);

        uint32_t checksum = ComputeChecksum(record);
        char checksum_buffer[4];
        EncodeFixed32(checksum_buffer, checksum);
        record.append(checksum_buffer, 4);

        write(fd_, record.data(), record.size());
    }

    void WAL::Sync()
    {
        fsync(fd_);
    }

    void WAL::Delete()
    {
        if (fd_ >= 0)
        {
            close(fd_);
            fd_ = -1;
        }
        unlink(path_.c_str());
    }

    bool WAL::Recover(RecoverCallback cb)
    {
        int read_fd = open(path_.c_str(), O_RDONLY);
        if (read_fd < 0)
            return true;

        std::string contents;
        char buffer[4096];
        ssize_t n;
        while ((n = read(read_fd, buffer, sizeof(buffer))) > 0)
        {
            contents.append(buffer, n);
        }
        close(read_fd);

        std::string_view input(contents);

        while (!input.empty())
        {
            if (input.size() < 14)
            {
                break;
            }

            const char *record_start = input.data();

            uint64_t sequence = DecodeFixed64(input.data());
            input.remove_prefix(8);

            std::string_view key;
            if (!GetLengthPrefixedSlice(&input, &key))
            {
                break;
            }

            std::string_view value;
            if (!GetLengthPrefixedSlice(&input, &value))
            {
                break;
            }

            if (input.size() < 4)
            {
                break;
            }
            uint32_t stored_checksum = DecodeFixed32(input.data());
            input.remove_prefix(4);

            size_t record_size = input.data() - record_start - 4;
            std::string record_bytes(record_start, record_size);
            uint32_t computed_checksum = ComputeChecksum(record_bytes);
            
            if (stored_checksum != computed_checksum)
            {
                break;
            }

            cb(sequence, key, value);
        }

        return true;
    }

    uint32_t WAL::ComputeChecksum(const std::string &data)
    {
        uint32_t checksum = 0;
        for (unsigned char c : data)
        {
            checksum ^= c;
        }
        return checksum;
    }

}
