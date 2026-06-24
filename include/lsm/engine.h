#pragma once
#include "lsm/memtable/memtable.h"
#include "lsm/wal/wal.h"
#include "lsm/util/lru_cache.h"
#include "lsm/compaction/level_manager.h"
#include "lsm/compaction/compaction_manager.h"
#include <string>
#include <string_view>
#include <cstdint>

namespace lsm
{
    class Engine
    {
    public:
        explicit Engine(const std::string &data_dir);
        ~Engine();

        bool Get(std::string_view key, std::string *value);
        void Put(std::string_view key, std::string_view value);

    private:
        void FlushMemTable();
        bool GetFromSSTable(const SSTableMeta &meta, std::string_view key, std::string *value);

        MemTable *memtable_;
        WAL *wal_;
        LRUCache *cache_;
        LevelManager *level_manager_;
        CompactionManager *compaction_manager_;

        uint64_t sequence_;
        uint64_t next_file_number_;
        std::string data_dir_;
    };

}
