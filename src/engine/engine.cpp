#include "engine/engine.hpp"
#include <mutex>
#include <chrono>

using namespace std;

namespace sapota {
   Engine::Engine(const string& wal_path) : log_(wal_path) {
        log_.replay([this](const string& op, const string& key, const string& value, time_t expiry) {
            if (op == "SET") {
                Entry e;
                e.value = value;
                if (expiry > 0) {
                    e.hasTTL = true;
                    e.expireAt = chrono::system_clock::from_time_t(expiry);
                    if (chrono::system_clock::now() >= e.expireAt) {
                        // Skip expired entries
                        return;
                    }
                }
                map_[key] = e;
            } else if (op == "DEL") {
                map_.erase(key);
            }
        });
}



    bool Engine::set(const string& key, const string& value, int ttlSeconds) {
        unique_lock lock(mtx_);
        if (!log_.append_set(key, value, ttlSeconds)) return false;

        Entry entry;
        entry.value = value;
        if (ttlSeconds > 0) {
            entry.hasTTL = true;
            entry.expireAt = chrono::system_clock::now() + chrono::seconds(ttlSeconds);
        }
        map_[key] = entry;
        return true;
    }


    optional<string> Engine::get(const string& key){
        unique_lock lock(mtx_);
        auto it = map_.find(key);
        if (it == map_.end()) return nullopt;

        if (it->second.hasTTL && chrono::system_clock::now() > it->second.expireAt) {
            map_.erase(it);
            return nullopt;
        }
        return it->second.value;
    }

    bool Engine::del(const string& key) {
        unique_lock lock(mtx_);
        if (!log_.append_del(key)) return false;
        return map_.erase(key) > 0;
    }

    vector<string> Engine::keys() {
        shared_lock lock(mtx_);
        vector<string> out;
        out.reserve(map_.size());

        for (auto it = map_.begin(); it != map_.end();) {
            if (it->second.hasTTL && chrono::system_clock::now() > it->second.expireAt) {
                it = map_.erase(it);
            } else {
                out.push_back(it -> first);
                ++it;
            }
        }
        return out;
    }

    void Engine::clear() {
        unique_lock lock(mtx_);
        map_.clear();
        // optional: truncate WAL here in future
    }
}
