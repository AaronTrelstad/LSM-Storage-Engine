#include "lsm/sstable/sstable_reader.h"
#include "lsm/util/coding.h"
#include "lsm/options.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

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

    bool SSTableReader::Get(const std::string_view key, std::string *value) const
    {
        std::string_view input(data_block_);

        while (!input.empty())
        {
            std::string_view entry_key;
            if (!GetLengthPrefixedSlice(&input, &entry_key))
                break;

            std::string_view entry_value;
            if (!GetLengthPrefixedSlice(&input, &entry_value))
                break;

            if (entry_key == key)
            {
                *value = std::string(entry_value);
                return true;
            }

            if (entry_key > key)
            {
                break;
            }
        }

        return false;
    }
}
