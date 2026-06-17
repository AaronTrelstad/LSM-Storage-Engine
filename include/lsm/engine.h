#pragma once
#include "lsm/memtable/memtable.h"
#include "lsm/wal/wal.h"
#include <string>

namespace lsm {
    class Engine {
        public:
            Engine(const std::string& data_dir);
            ~Engine();

            bool Get(std::string_view key, std::string* value);
            void Put(std::string_view key, std::string_view value);
        private:
            void FlushMemTable();

            MemTable* memtable_;
            WAL* wal_;
            uint64_t sequence_;
            std::string data_dir_;

    };
}
