#include "lsm/engine.h"
#include "lsm/wal/wal.h"
#include "lsm/memtable/memtable.h"
#include "lsm/options.h"

namespace lsm
{
    Engine::Engine(const std::string &data_dir) : data_dir_(data_dir), sequence_(0) 
    {
        memtable_ = new MemTable(kMemTableSize);
        wal_ = new WAL(data_dir_ + kWALFileName);

        wal_->Recover([&](uint64_t seq, std::string_view key, std::string_view value) {
            memtable_->Put(key, value);
            sequence_ = std::max(sequence_, seq); 
        });
    }

    Engine::~Engine()
    {
        if (memtable_->MemoryUsage() > 0)
        {
            FlushMemTable();
        }

        delete memtable_;
        delete wal_;
    }

    bool Engine::Get(std::string_view key, std::string *value)
    {
        if (memtable_->Get(key, value))
        {
            return true;
        }

        return false;
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
        wal_->Delete();
    }
}
