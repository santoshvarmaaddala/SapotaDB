#include "tx/tx.hpp"

using namespace sapota;

TxnManager::TxnManager() = default;

Txn TxnManager::begin() {
    TxnId id = next_txn_id_.fetch_add(1);
    CommitTs snap = global_ts_.load();
    return Txn{id, snap};
}

CommitTs TxnManager::next_commit_ts() {
    // increment and return
    return global_ts_.fetch_add(1) + 1;
}
