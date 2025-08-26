#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include "engine/engine.hpp"

using namespace sapota;

static std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    return s.substr(i, j - i);
}

static std::vector<std::string> split_once(const std::string& s) {
    std::vector<std::string> parts;
    std::istringstream iss(s);
    std::string cmd;
    if (!(iss >> cmd)) return parts;
    parts.push_back(cmd);
    std::string rest;
    std::getline(iss, rest);
    rest = trim(rest);
    if (!rest.empty()) parts.push_back(rest);
    return parts;
}

int main() {
    Engine db("./sapota_wal.log");

    std::cout << "\n 🌱 SapotaDB (MVCC) ";
    std::cout << "\n Commands:\n  SET <key> <value> [ttl_seconds]\n  GET <key>\n  DELETE <key>\n  KEYS\n  BEGIN | COMMIT | ABORT (transactional usage TBD)\n  EXIT\n\n";

    std::string line;
    std::optional<Txn> cur;

    while (true) {
        std::cout << "sapota> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;
        auto parts = split_once(line);
        if (parts.empty()) continue;

        std::string cmd = parts[0];
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c){ return std::toupper(c); });

        if (cmd == "EXIT" || cmd == "QUIT") break;
        else if (cmd == "SET") {
            if (parts.size() < 2) { std::cout << "ERR missing arguments\n"; continue; }
            std::istringstream iss(parts[1]);
            std::string key;
            if (!(iss >> key)) { std::cout << "ERR missing key\n"; continue; }
            std::string value;
            if (!(iss >> std::quoted(value))) {
                std::getline(iss, value);
                value = trim(value);
            }
            if (value.empty()) { std::cout << "ERR missing value\n"; continue; }
            int ttl = 0; iss >> ttl;
            bool ok = db.set(key, value, ttl);
            std::cout << (ok ? "OK\n" : "ERR\n");
        }
        else if (cmd == "GET") {
            if (parts.size() != 2) { std::cout << "ERR usage: GET <key>\n"; continue; }
            auto val = db.get(parts[1]);
            if (!val) std::cout << "(nil)\n"; else std::cout << *val << "\n";
        }
        else if (cmd == "DELETE" || cmd == "DEL") {
            if (parts.size() != 2) { std::cout << "ERR usage: DELETE <key>\n"; continue; }
            bool ok = db.del(parts[1]);
            std::cout << (ok ? "OK\n" : "ERR\n");
        }
        else if (cmd == "KEYS") {
            auto ks = db.keys();
            if (ks.empty()) std::cout << "(empty)\n";
            else for (auto &k : ks) std::cout << k << "\n";
        }
        else if (cmd == "BEGIN") {
            cur = db.begin();
            std::cout << "TXN STARTED id=" << cur->id << " snap=" << cur->snapshot_ts << "\n";
        }
        else if (cmd == "COMMIT") {
            if (!cur) { std::cout << "ERR not in txn\n"; continue; }
            bool ok = db.commit(*cur);
            std::cout << (ok ? "COMMIT OK\n" : "COMMIT FAIL (conflict)\n");
            cur.reset();
        }
        else if (cmd == "ABORT") {
            if (!cur) { std::cout << "ERR not in txn\n"; continue; }
            db.abort(*cur);
            cur.reset();
            std::cout << "ABORTED\n";
        }
        else {
            std::cout << "ERR unknown command\n";
        }
    }
    return 0;
}
