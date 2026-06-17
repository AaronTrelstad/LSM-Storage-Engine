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
    }

    SSTableReader::~SSTableReader() {
        if (fd_ >= 0) {
            close(fd_);
        }
    }
}
