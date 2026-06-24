#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace lsm
{

    struct SSTableMeta
    {
        std::string path;
        std::string smallest_key;
        std::string largest_key;
        uint64_t file_size = 0;
    };

    class LevelManager
    {
    public:
        LevelManager(const std::string &data_dir, int max_levels);

        void AddSSTable(int level, SSTableMeta meta);
        void RemoveSSTable(int level, const std::string &path);

        const std::vector<SSTableMeta> &GetLevel(int level) const;
        size_t NumFiles(int level) const;
        int NumLevels() const { return max_levels_; }
        uint64_t LevelBytes(int level) const;

        void Save() const;
        void Load();

    private:
        std::string data_dir_;
        int max_levels_;
        std::vector<std::vector<SSTableMeta>> levels_;
    };

}
