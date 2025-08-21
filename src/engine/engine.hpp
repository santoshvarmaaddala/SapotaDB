#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <shared_mutex>
#include <chrono>
#include "log/log_manager.hpp"

using namespace std;

namespace sapota {

    struct Entry {
        string value;
        bool hasTTL = false;
        chrono::steady_clock::time_point expireAt;
    };

    class Engine {
    public:
        Engine(const string& wal_path);
        Engine() : Engine("wal.log") {}   
        ~Engine() = default;
    
        bool set(const string& key, const string& value, int ttlSeconds = -1);
        optional<string> get(const string& key);
        bool del(const string& key);
        vector<string> keys();
        void clear();

    private:
        mutable shared_mutex mtx_;
        unordered_map<string, Entry> map_;
        LogManager log_;
    };
}
