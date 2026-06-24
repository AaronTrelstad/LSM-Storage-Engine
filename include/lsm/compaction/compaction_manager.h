#pragma once
#include "lsm/compaction/level_manager.h"
#include "lsm/util/lru_cache.h"
#include <string>
#include <array>
#include <cstdint>

namespace lsm
{
    class CompactionManager
    {
    public:
        CompactionManager(LevelManager *level_manager, LRUCache *cache,
                          const std::string &data_dir);

        bool MaybeCompact();

    private:
        void CompactLevel(int level);
        std::string NewSSTablePath(int level);

        LevelManager *level_manager_;
        LRUCache *cache_;
        std::string data_dir_;
        uint64_t next_file_number_;

        static constexpr size_t kLevel0CompactionTrigger = 4;

        static constexpr std::array<uint64_t, 6> kLevelMaxBytes = {{
            10ULL * 1024 * 1024,
            100ULL * 1024 * 1024,
            1000ULL * 1024 * 1024,
            10000ULL * 1024 * 1024,
            100000ULL * 1024 * 1024,
            1000000ULL * 1024 * 1024,
        }};
    };

}
