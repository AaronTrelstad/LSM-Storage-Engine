#include "lsm/compaction/level_manager.h"
#include "lsm/util/coding.h"

#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace lsm
{

    LevelManager::LevelManager(const std::string &data_dir, int max_levels)
        : data_dir_(data_dir), max_levels_(max_levels), levels_(max_levels) {}

    void LevelManager::AddSSTable(int level, SSTableMeta meta)
    {
        levels_[level].push_back(std::move(meta));
    }

    void LevelManager::RemoveSSTable(int level, const std::string &path)
    {
        auto &vec = levels_[level];
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                                 [&](const SSTableMeta &m)
                                 { return m.path == path; }),
                  vec.end());
    }

    const std::vector<SSTableMeta> &LevelManager::GetLevel(int level) const
    {
        return levels_[level];
    }

    size_t LevelManager::NumFiles(int level) const
    {
        return levels_[level].size();
    }

    uint64_t LevelManager::LevelBytes(int level) const
    {
        uint64_t total = 0;
        for (const auto &m : levels_[level])
            total += m.file_size;
        return total;
    }

    void LevelManager::Save() const
    {
        std::string manifest_path = data_dir_ + "/MANIFEST";
        std::ofstream out(manifest_path, std::ios::binary | std::ios::trunc);
        if (!out)
            throw std::runtime_error("LevelManager: cannot write manifest");

        auto write_str = [&](const std::string &s)
        {
            char len_buf[4];
            EncodeFixed32(len_buf, static_cast<uint32_t>(s.size()));
            out.write(len_buf, 4);
            out.write(s.data(), static_cast<std::streamsize>(s.size()));
        };

        for (int level = 0; level < max_levels_; level++)
        {
            for (const auto &meta : levels_[level])
            {
                char hdr[12];
                EncodeFixed32(hdr, static_cast<uint32_t>(level));
                EncodeFixed64(hdr + 4, meta.file_size);
                out.write(hdr, 12);
                write_str(meta.path);
                write_str(meta.smallest_key);
                write_str(meta.largest_key);
            }
        }
    }

    void LevelManager::Load()
    {
        std::string manifest_path = data_dir_ + "/MANIFEST";
        std::ifstream in(manifest_path, std::ios::binary);
        if (!in)
            return;

        levels_.assign(max_levels_, {});

        auto read_str = [&]() -> std::string
        {
            char len_buf[4];
            if (!in.read(len_buf, 4))
                return {};
            uint32_t len = DecodeFixed32(len_buf);
            std::string s(len, '\0');
            in.read(s.data(), static_cast<std::streamsize>(len));
            return s;
        };

        char hdr[12];
        while (in.read(hdr, 12))
        {
            int level = static_cast<int>(DecodeFixed32(hdr));
            uint64_t file_size = DecodeFixed64(hdr + 4);
            if (level < 0 || level >= max_levels_)
                break;

            SSTableMeta meta;
            meta.file_size = file_size;
            meta.path = read_str();
            meta.smallest_key = read_str();
            meta.largest_key = read_str();

            if (in)
                levels_[level].push_back(std::move(meta));
        }
    }

}
