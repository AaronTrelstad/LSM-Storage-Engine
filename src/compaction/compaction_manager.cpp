#include "lsm/compaction/compaction_manager.h"
#include "lsm/sstable/sstable_reader.h"
#include "lsm/sstable/sstable_builder.h"

#include <map>
#include <chrono>
#include <filesystem>
#include <stdexcept>

namespace lsm
{

    CompactionManager::CompactionManager(LevelManager *level_manager, LRUCache *cache,
                                         const std::string &data_dir)
        : level_manager_(level_manager), cache_(cache),
          data_dir_(data_dir), next_file_number_(0) {}

    std::string CompactionManager::NewSSTablePath(int level)
    {
        auto ns = std::chrono::steady_clock::now().time_since_epoch().count();
        return data_dir_ + "/L" + std::to_string(level) + "_c" +
               std::to_string(ns) + "_" + std::to_string(next_file_number_++) + ".sst";
    }

    bool CompactionManager::MaybeCompact()
    {
        bool did_compact = false;

        if (level_manager_->NumFiles(0) >= kLevel0CompactionTrigger)
        {
            CompactLevel(0);
            did_compact = true;
        }

        for (int level = 1; level < level_manager_->NumLevels() - 1; level++)
        {
            auto idx = static_cast<size_t>(level - 1);
            if (idx < kLevelMaxBytes.size() &&
                level_manager_->LevelBytes(level) > kLevelMaxBytes[idx])
            {
                CompactLevel(level);
                did_compact = true;
            }
        }

        return did_compact;
    }

    void CompactionManager::CompactLevel(int level)
    {
        int target = level + 1;
        if (target >= level_manager_->NumLevels())
            return;

        std::map<std::string, std::string> merged;

        for (const auto &meta : level_manager_->GetLevel(target))
        {
            SSTableReader reader(meta.path);
            reader.ForEach([&](std::string_view k, std::string_view v)
                           { merged.emplace(std::string(k), std::string(v)); });
        }

        for (const auto &meta : level_manager_->GetLevel(level))
        {
            SSTableReader reader(meta.path);
            reader.ForEach([&](std::string_view k, std::string_view v)
                           { merged[std::string(k)] = std::string(v); });
        }

        std::vector<std::string> to_delete;
        for (const auto &meta : level_manager_->GetLevel(target))
            to_delete.push_back(meta.path);
        for (const auto &meta : level_manager_->GetLevel(level))
            to_delete.push_back(meta.path);

        SSTableMeta new_meta;
        if (!merged.empty())
        {
            std::string out_path = NewSSTablePath(target);
            SSTableBuilder builder(out_path);
            bool first = true;
            uint64_t file_size = 0;

            for (const auto &[k, v] : merged)
            {
                if (first)
                {
                    new_meta.smallest_key = k;
                    first = false;
                }
                new_meta.largest_key = k;
                file_size += static_cast<uint64_t>(k.size() + v.size() + 8);
                builder.Add(k, v);
            }
            builder.Finish();

            new_meta.path = out_path;
            new_meta.file_size = file_size;
        }

        while (level_manager_->NumFiles(target) > 0)
            level_manager_->RemoveSSTable(target, level_manager_->GetLevel(target)[0].path);
        while (level_manager_->NumFiles(level) > 0)
            level_manager_->RemoveSSTable(level, level_manager_->GetLevel(level)[0].path);

        for (const auto &path : to_delete)
        {
            cache_->Erase(path);
            std::filesystem::remove(path);
        }

        if (!merged.empty())
        {
            level_manager_->AddSSTable(target, std::move(new_meta));
        }

        level_manager_->Save();
    }

}
