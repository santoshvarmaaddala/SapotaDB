#include "log/log_manager.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

using namespace sapota;

LogManager::LogManager(const std::string& wal_path) : wal_path_(wal_path) {
    // ensure parent dir exists (no-op if path has no parent)
    std::filesystem::path p(wal_path_);
    if (!p.parent_path().empty()) std::filesystem::create_directories(p.parent_path());
}

LogManager::~LogManager() = default;

bool LogManager::append_set(const std::string& key, const std::string& value, std::time_t expiry) {
    std::ofstream ofs(wal_path_, std::ios::app | std::ios::binary);
    if (!ofs.is_open()) return false;
    ofs << "SET " << key.size() << " " << value.size() << " " << expiry << "\n";
    ofs << key << value << "\n";
    ofs.flush();
    return true;
}

bool LogManager::append_del(const std::string& key) {
    std::ofstream ofs(wal_path_, std::ios::app | std::ios::binary);
    if (!ofs.is_open()) return false;
    ofs << "DEL " << key.size() << "\n";
    ofs << key << "\n";
    ofs.flush();
    return true;
}

size_t LogManager::replay(const std::function<void(const std::string&, const std::string&, const std::string&, std::time_t)>& cb) {
    std::ifstream ifs(wal_path_, std::ios::binary);
    if (!ifs.is_open()) return 0;

    size_t count = 0;
    std::string op;
    while (ifs >> op) {
        if (op == "SET") {
            size_t klen = 0, vlen = 0; std::time_t expiry = 0;
            if (!(ifs >> klen >> vlen >> expiry)) break;
            ifs.get(); // consume newline
            std::string kv(klen + vlen, '\0');
            if (!ifs.read(&kv[0], klen + vlen)) break;
            ifs.get(); // consume newline
            std::string key = kv.substr(0, klen);
            std::string val = kv.substr(klen, vlen);
            cb("SET", key, val, expiry);
            ++count;
        } else if (op == "DEL") {
            size_t klen = 0;
            if (!(ifs >> klen)) break;
            ifs.get(); // consume newline
            std::string key(klen, '\0');
            if (!ifs.read(&key[0], klen)) break;
            ifs.get(); // consume newline
            cb("DEL", key, std::string(), 0);
            ++count;
        } else {
            std::cerr << "[WAL] Unknown op '" << op << "' while replaying. Stop.\n";
            break;
        }
    }
    return count;
}
