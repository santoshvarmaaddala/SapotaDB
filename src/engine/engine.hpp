#pragma once
#include <string>
#include <vector>
#include <optional>
#include "log/log_manager.hpp"
#include "storage/mvcc.hpp"
#include "tx/tx.hpp"

namespace sapota {

class Engine {
public:
    explicit Engine(const std::string& wal_path);
    ~Engine() = default;

    // autocommit APIs
    bool set(const std::string& key, const std::string& value, int ttlSeconds = 0);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    std::vector<std::string> keys();

    // explicit transactional API
    Txn begin();
    bool commit(const Txn& txn);
    void abort(const Txn& txn);

    // Transactional staging APIs (used by CLI when a Txn is active)
    // Stage a write inside an existing transaction (no commit, durable on COMMIT)
    void stage_set(const Txn& txn, const std::string& key, const std::string& value, int ttlSeconds = 0);

    // Stage a delete inside an existing transaction
    void stage_del(const Txn& txn, const std::string& key);

    // Read inside a transaction: check staged writes first, then snapshot read
    std::optional<std::string> get(const Txn& txn, const std::string& key);


private:
    LogManager log_;
    MVCCStore store_;
    TxnManager txmgr_;
};

} // namespace sapota
