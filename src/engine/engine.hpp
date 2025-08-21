#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <shared_mutex>
#include "log/log_manager.hpp"

using namespace std;

namespace sapota {
    class Engine {
    public:
        Engine(const string& wal_path);
        Engine() : Engine("wal.log") {}   
        ~Engine() = default;
    
        bool put(const string& key, const string& value);
        optional<string> get(const string& key) const;
        bool del(const string& key);
        vector<string> keys() const;
        void clear();

    private:
        mutable shared_mutex mtx_;
        unordered_map<string, string> map_;
        LogManager log_;
    };
}
