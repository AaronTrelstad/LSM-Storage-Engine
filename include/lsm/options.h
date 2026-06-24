#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace lsm
{
    inline constexpr size_t kMemTableSize = 64 * 1024 * 1024;
    inline constexpr int kMaxSkipListLevel = 12;
    inline constexpr const char *kWALFileName = "/wal.log";
    inline constexpr uint64_t kSSTableMagicNumber = 0xDEADBEEFCAFEF00DULL;
    inline constexpr size_t kSSTableFooterSize = 24;

    inline constexpr size_t kBlockCacheCapacity = 128;

    inline constexpr int kNumLevels = 7;
}
