#pragma once
#include <string>
#include <functional>
#include <ctime>

namespace sapota {

class LogManager {
public:
    explicit LogManager(const std::string& wal_path);
    ~LogManager();

    // emit a SET record: key/value sizes and expiry epoch seconds (0 = none)
    bool append_set(const std::string& key, const std::string& value, std::time_t expiry);

    // emit a DEL record
    bool append_del(const std::string& key);

    // Replay WAL. Callback: op ("SET"/"DEL"), key, value (empty for DEL), expiry (0 if none)
    size_t replay(const std::function<void(const std::string&, const std::string&, const std::string&, std::time_t)>& cb);

private:
    std::string wal_path_;
};

} // namespace sapota
