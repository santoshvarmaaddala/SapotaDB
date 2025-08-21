#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "engine/engine.hpp"

using namespace sapota;


static std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    return s.substr(i, j - i);
}

static std::vector<std::string> split_once(const std::string& s) {
    // Split into cmd and the rest (payload) so values can contain spaces.
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
    Engine db;

    std::cout << "\n 🌱 Welcome to SapotaDB ";
    std::cout << "\n Commands:\n  PUT <key> <value>\n  GET <key>\n  DELETE <key>\n  KEYS\n  EXIT\n\n";

    std::string line;
    while (true) {
        std::cout << "sapota> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;

        auto parts = split_once(line);
        if (parts.empty()) continue;

        std::string cmd = parts[0];
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c){ return std::toupper(c); });

        if (cmd == "EXIT" || cmd == "QUIT") {
            std::cout << "Thanks for Tasting" << std::endl;
            break;
        }
        else if (cmd == "PUT") {
            // Expect: PUT <key> <value>
            if (parts.size() < 2) { std::cout << "ERR missing arguments" << std::endl; continue; }
            // Extract key (first token of the payload) and value (rest)
            std::istringstream iss(parts[1]);
            std::string key; if (!(iss >> key)) { std::cout << "ERR missing key" << std::endl; continue; }
            std::string value; std::getline(iss, value); value = trim(value);
            if (value.empty()) { std::cout << "ERR missing value" << std::endl; continue; }
            db.put(key, value);
            std::cout << "Inserted" << std::endl;
        }
        else if (cmd == "GET") {
            if (parts.size() != 2) { std::cout << "ERR usage: GET <key>" << std::endl; continue; }
            auto key = parts[1];
            auto val = db.get(key);
            if (!val) std::cout << "Not Found" << std::endl;
            else std::cout << *val << std::endl;
        }
        else if (cmd == "DELETE" || cmd == "DEL") {
            if (parts.size() != 2) { std::cout << "ERR usage: DELETE <key>" << std::endl; continue; }
            bool ok = db.del(parts[1]);
            std::cout << (ok ? "Deleted" : "No key found or not able to delete") << std::endl;
        }
        else if (cmd == "KEYS") {
            auto ks = db.keys();
            size_t i, len = ks.size();
            if (len == 0) {
                std::cout << "No stored keys" << std::endl;
            } else {
                for (i = 0; i < len; ++i) {
                    std::cout << ks[i] << std::endl;
                }
            }
            
        }
        else {
            std::cout << "ERR unknown command" << std::endl;
        }
    }

    return 0;
    }