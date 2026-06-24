#include "lsm/engine.h"
#include "lsm/wal/wal.h"
#include "lsm/memtable/memtable.h"
#include "lsm/sstable/sstable_builder.h"
#include "lsm/sstable/sstable_reader.h"
#include "lsm/options.h"

#include <chrono>
#include <filesystem>

namespace lsm
{
    Engine::Engine(const std::string &data_dir)
        : data_dir_(data_dir), sequence_(0), next_file_number_(0)
    {
        std::filesystem::create_directories(data_dir_);

        cache_ = new LRUCache(kBlockCacheCapacity);
        level_manager_ = new LevelManager(data_dir_, kNumLevels);
        level_manager_->Load();
        compaction_manager_ = new CompactionManager(level_manager_, cache_, data_dir_);

        memtable_ = new MemTable(kMemTableSize);
        wal_ = new WAL(data_dir_ + kWALFileName);

        wal_->Recover([&](uint64_t seq, std::string_view key, std::string_view value)
                      {
            memtable_->Put(key, value);
            sequence_ = std::max(sequence_, seq); });
    }

    Engine::~Engine()
    {
        if (memtable_->MemoryUsage() > 0)
        {
            FlushMemTable();
        }

        delete memtable_;
        delete wal_;
        delete compaction_manager_;
        delete level_manager_;
        delete cache_;
    }

    bool Engine::Get(std::string_view key, std::string *value)
    {
        if (memtable_->Get(key, value))
            return true;

        const auto &l0 = level_manager_->GetLevel(0);
        for (int i = static_cast<int>(l0.size()) - 1; i >= 0; i--)
        {
            if (GetFromSSTable(l0[i], key, value))
                return true;
        }

        for (int level = 1; level < level_manager_->NumLevels(); level++)
        {
            for (const auto &meta : level_manager_->GetLevel(level))
            {
                if (key < meta.smallest_key || key > meta.largest_key)
                    continue;
                if (GetFromSSTable(meta, key, value))
                    return true;
            }
        }

        return false;
    }

    bool Engine::GetFromSSTable(const SSTableMeta &meta, std::string_view key, std::string *value)
    {
        std::string block;
        if (!cache_->Get(meta.path, &block))
        {
            SSTableReader reader(meta.path);
            block = reader.GetDataBlock();
            cache_->Put(meta.path, block);
        }
        return SSTableReader::SearchBlock(block, key, value);
    }

    void Engine::Put(std::string_view key, std::string_view value)
    {
        if (!memtable_->CanFit(key.size() + value.size()))
        {
            FlushMemTable();
            delete memtable_;
            delete wal_;

            memtable_ = new MemTable(kMemTableSize);
            wal_ = new WAL(data_dir_ + kWALFileName);
        }

        sequence_++;
        wal_->Append(sequence_, key, value);
        wal_->Sync();
        memtable_->Put(key, value);
    }

    void Engine::FlushMemTable()
    {
        auto ns = std::chrono::steady_clock::now().time_since_epoch().count();
        std::string path = data_dir_ + "/L0_" + std::to_string(ns) + "_" +
                           std::to_string(next_file_number_++) + ".sst";

        SSTableBuilder builder(path);

        SSTableMeta meta;
        meta.path = path;
        bool first = true;
        uint64_t file_size = 0;

        auto it = memtable_->NewIterator();
        it.SeekToFirst();
        while (it.Valid())
        {
            if (first)
            {
                meta.smallest_key = std::string(it.key());
                first = false;
            }
            meta.largest_key = std::string(it.key());
            file_size += static_cast<uint64_t>(it.key().size() + it.value().size() + 8);
            builder.Add(it.key(), it.value());
            it.Next();
        }
        builder.Finish();
        meta.file_size = file_size;

        if (!first)
        {
            level_manager_->AddSSTable(0, std::move(meta));
            level_manager_->Save();
            compaction_manager_->MaybeCompact();
        }

        wal_->Delete();
    }
}
