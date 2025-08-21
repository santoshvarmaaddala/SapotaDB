#include "log/log_manager.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

using namespace std;

LogManager::LogManager(const string& wal_path) : wal_path_(wal_path) {
    // Ensure parent directory exists
    filesystem::create_directories(filesystem::path(wal_path_).parent_path());
}

LogManager::~LogManager() {}

bool LogManager::append_set(const string& key, const string& value) {
    ofstream ofs(wal_path_, ios::app | ios::binary);
    if (!ofs.is_open()) return false;
    ofs << "SET " << key.size() << " " << value.size() << "\n";
    ofs << key << value << "\n";
    ofs.flush();
    return true;
}

bool LogManager::append_del(const string& key) {
    ofstream ofs(wal_path_, ios::app | ios::binary);
    if (!ofs.is_open()) return false;
    ofs << "DEL " << key.size() << "\n";
    ofs << key << "\n";
    ofs.flush();
    return true;
}

size_t LogManager::replay(
    const function<void(const string&, const string&, const string&)>& cb
) {
    ifstream ifs(wal_path_, ios::binary);
    if (!ifs.is_open()) return 0;

    size_t count = 0;
    string op;
    while (ifs >> op) {
        if (op == "SET") {
            size_t klen, vlen;
            ifs >> klen >> vlen;
            ifs.get(); // consume newline
            string kv(klen + vlen, '\0');
            ifs.read(&kv[0], klen + vlen);
            ifs.get(); // consume newline
            string key = kv.substr(0, klen);
            string value = kv.substr(klen, vlen);
            cb("SET", key, value);
            count++;
        } else if (op == "DEL") {
            size_t klen;
            ifs >> klen;
            ifs.get(); // consume newline
            string key(klen, '\0');
            ifs.read(&key[0], klen);
            ifs.get(); // consume newline
            cb("DEL", key, "");
            count++;
        } else {
            // unknown op → skip
            break;
        }
    }
    return count;
}
