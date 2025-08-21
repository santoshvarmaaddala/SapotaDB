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

bool LogManager::append_set(const string& key, const string& value, int ttlSeconds) {
    ofstream ofs(wal_path_, ios::app | ios::binary);
    if (!ofs.is_open()) return false;

    // If no TTL, mark expiry as 0
    long long expireAt = 0;
    if (ttlSeconds > 0) {
        auto now = chrono::system_clock::now();
        expireAt = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count() + ttlSeconds;
    }

    ofs << "SET " << key.size() << " " << value.size() << " " << expireAt << "\n";
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
    const function<void(const string&, const string&, const string&, time_t)>& cb
) {
    ifstream ifs(wal_path_, ios::binary);
    if (!ifs.is_open()) return 0;

    size_t count = 0;
    string op;
    while (ifs >> op) {
        if (op == "SET") {
            size_t klen, vlen;
            time_t expiry;
            if (!(ifs >> klen >> vlen >> expiry)) break;
            ifs.get(); // consume newline

            string kv(klen + vlen, '\0');
            if (!ifs.read(&kv[0], klen + vlen)) break;
            ifs.get(); // consume newline

            string key = kv.substr(0, klen);
            string value = kv.substr(klen, vlen);

            cb("SET", key, value, expiry);
            count++;
        } 
        else if (op == "DEL") {
            size_t klen;
            if (!(ifs >> klen)) break;
            ifs.get(); // consume newline

            string key(klen, '\0');
            if (!ifs.read(&key[0], klen)) break;
            ifs.get(); // consume newline

            cb("DEL", key, "", 0);
            count++;
        } 
        else {
            cerr << "[WAL] Unknown op in log, stopping replay.\n";
            break;
        }
    }
    return count;
}


