#pragma once
#include <string>
#include <functional>

using namespace std;

class LogManager {
    public:
        explicit LogManager(const string& wal_path);
        ~LogManager();

        bool append_set(const string& key, const string& value);
        bool append_del(const string& key);

        size_t replay(
            const function<void(const string&, const string&, const string&)>& cb
        );

    private:
        string wal_path_;
};
