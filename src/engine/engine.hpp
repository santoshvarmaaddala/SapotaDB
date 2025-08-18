#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <shared_mutex>


namespace sapota {
    class Engine {
        public:
            Engine() = default;
            ~Engine() = default;
        
            bool put (const std::string& key, const std::string& value);

            std::optional<std::string> get(const std::string& key) const;

            bool del(const std::string& key);

            std::vector<std::string> keys() const;

            void clear();
        
        private:
            mutable std::shared_mutex mtx_;
            std::unordered_map<std::string, std::string> map_;  
    }
}