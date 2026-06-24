#include "lsm/sstable/sstable_reader.h"
#include "lsm/util/coding.h"
#include "lsm/options.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <functional>

namespace lsm
{
    SSTableReader::SSTableReader(const std::string &path) : path_(path), fd_(-1), data_block_size_(0), filter_block_size_(0)
    {
        fd_ = open(path.c_str(), O_RDONLY);
        if (fd_ < 0)
        {
            throw std::runtime_error("SSTableReader failed to open file: " + path);
        }

        off_t file_size = lseek(fd_, 0, SEEK_END);
        if (file_size < static_cast<off_t>(kSSTableFooterSize))
        {
            throw std::runtime_error("SSTableReader file too small to be valid: " + path);
        }

        char footer[kSSTableFooterSize];
        lseek(fd_, file_size - kSSTableFooterSize, SEEK_SET);
        read(fd_, footer, kSSTableFooterSize);

        data_block_size_ = DecodeFixed64(footer);
        filter_block_size_ = DecodeFixed64(footer + 8);
        uint64_t magic = DecodeFixed64(footer + 16);

        if (magic != kSSTableMagicNumber)
        {
            throw std::runtime_error("SSTableReader corrupt file: " + path);
        }

        data_block_.resize(data_block_size_);
        lseek(fd_, 0, SEEK_SET);
        read(fd_, data_block_.data(), data_block_size_);
    }

    SSTableReader::~SSTableReader()
    {
        if (fd_ >= 0)
        {
            close(fd_);
        }
    }

    bool SSTableReader::Get(std::string_view key, std::string *value) const
    {
        return SearchBlock(data_block_, key, value);
    }

    void SSTableReader::ForEach(std::function<void(std::string_view, std::string_view)> cb) const
    {
        std::string_view input(data_block_);
        std::string_view k, v;
        while (GetLengthPrefixedSlice(&input, &k) && GetLengthPrefixedSlice(&input, &v))
        {
            cb(k, v);
        }
    }

    bool SSTableReader::SearchBlock(std::string_view block, std::string_view key, std::string *value)
    {
        std::string_view input(block);
        std::string_view entry_key, entry_value;
        while (GetLengthPrefixedSlice(&input, &entry_key) &&
               GetLengthPrefixedSlice(&input, &entry_value))
        {
            if (entry_key == key)
            {
                *value = std::string(entry_value);
                return true;
            }
            if (entry_key > key)
                break;
        }
        return false;
    }
}
