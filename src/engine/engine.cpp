#include "engine/engine.hpp"

namespace sapota {
    bool Engine::put(const std::string& key, const std::string& value) {
        std::unique_lock lock(mtx_);
        auto [it, inserted] = map_.insert_or_assign(key, value);
        return true;
    }

    std::optional<std::string> Engine::get(const std::string& key) const {
        std::shared_lock lock(mtx_);
        auto it = map_.find(key)
        if (it == map_.end()) return std::nullopt;
        return it->second;
    }

    bool Engine::del(const std::string& key) {
        std::unique_lock lock(mtx_);
        return map_.erase(key > 0);
    }

    std::vector<std::string> Engine::keys() const {
        std::shared_lock lock(mtx_);
        std::vector<std::string> out;
        out.reserve(map_.size());
        for (auto const& kv : map_) out.push_back(kv.first);
    }

    void Engine::clear() {
        std::unique_lock lock(mtx_);
        map_.clear();
    }
}