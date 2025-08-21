#include "engine/engine.hpp"
#include <mutex>

using namespace std;

namespace sapota {
    Engine::Engine(const string& wal_path) : log_(wal_path) {
        // Replay WAL into map_ at startup
        log_.replay([this](const string& op, const string& key, const string& value) {
            if (op == "SET") {
                map_[key] = value;
            } else if (op == "DEL") {
                map_.erase(key);
            }
        });
    }

    bool Engine::put(const string& key, const string& value) {
        unique_lock lock(mtx_);
        if (!log_.append_set(key, value)) return false;
        map_[key] = value;
        return true;
    }

    optional<string> Engine::get(const string& key) const {
        shared_lock lock(mtx_);
        auto it = map_.find(key);
        if (it == map_.end()) return nullopt;
        return it->second;
    }

    bool Engine::del(const string& key) {
        unique_lock lock(mtx_);
        if (!log_.append_del(key)) return false;
        return map_.erase(key) > 0;
    }

    vector<string> Engine::keys() const {
        shared_lock lock(mtx_);
        vector<string> out;
        out.reserve(map_.size());
        for (auto const& kv : map_) out.push_back(kv.first);
        return out;
    }

    void Engine::clear() {
        unique_lock lock(mtx_);
        map_.clear();
        // optional: truncate WAL here in future
    }
}
