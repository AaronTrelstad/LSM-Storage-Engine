#include "lsm/sstable/sstable_builder.h"
#include "lsm/util/coding.h"
#include "lsm/options.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

namespace lsm
{
    SSTableBuilder::SSTableBuilder(const std::string &path)
        : path_(path), fd_(-1), data_block_size_(0)
    {
        fd_ = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd_ < 0)
        {
            throw std::runtime_error("SSTableBuilder failed to open file: " + path);
        }
    }

    SSTableBuilder::~SSTableBuilder()
    {
        if (fd_ >= 0)
        {
            close(fd_);
        }
    }

    void SSTableBuilder::Add(std::string_view key, std::string_view value)
    {
        PutLengthPrefixedSlice(&data_buf_, key);
        PutLengthPrefixedSlice(&data_buf_, value);
        keys_.emplace_back(key);
    }

    void SSTableBuilder::Finish()
    {
        data_block_size_ = data_buf_.size();
        write(fd_, data_buf_.data(), data_buf_.size());

        uint64_t filter_block_size = 0; 

        char footer[kSSTableFooterSize];
        EncodeFixed64(footer, data_block_size_);
        EncodeFixed64(footer + 8, filter_block_size);
        EncodeFixed64(footer + 16, kSSTableMagicNumber);

        write(fd_, footer, kSSTableFooterSize);

        close(fd_);
        fd_ = -1;
    }
}
