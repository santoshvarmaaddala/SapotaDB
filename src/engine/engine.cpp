#include "engine/engine.hpp"
#include <chrono>

using namespace sapota;

static inline std::time_t now_epoch() {
    return static_cast<std::time_t>(std::time(nullptr));
}

Engine::Engine(const std::string& wal_path)
    : log_(wal_path), store_(), txmgr_()
{
    // replay WAL: install as successive committed versions
    // We'll assign increasing commit ts starting from txmgr_.next_commit_ts()
    log_.replay([this](const std::string& op, const std::string& key, const std::string& value, std::time_t expiry) {
        // stage under synthetic txn id 0
        // We'll use commit timestamp from txmgr_
        TxnId synthetic_txn = 0;
        // Simplest: stage and immediately commit under incremented commit ts
        store_.stage_set(1, key, value, expiry);
        CommitTs cts = txmgr_.next_commit_ts();
        // commit with read_snapshot = cts-1 to avoid conflicts
        store_.commit(1, cts, (cts > 0 ? cts - 1 : 0));
    });
}

bool Engine::set(const std::string& key, const std::string& value, int ttlSeconds) {
    Txn txn = txmgr_.begin();
    std::time_t expiry = 0;
    if (ttlSeconds > 0) expiry = now_epoch() + ttlSeconds;
    store_.stage_set(txn.id, key, value, expiry);
    CommitTs cts = txmgr_.next_commit_ts();
    bool ok = store_.commit(txn.id, cts, txn.snapshot_ts);
    if (!ok) {
        store_.abort(txn.id);
        return false;
    }
    // durable
    log_.append_set(key, value, expiry);
    return true;
}

std::optional<std::string> Engine::get(const std::string& key) {
    // read at current global timestamp snapshot
    CommitTs snap = txmgr_.next_commit_ts() - 1;
    return store_.get(key, snap, now_epoch());
}

bool Engine::del(const std::string& key) {
    Txn txn = txmgr_.begin();
    store_.stage_del(txn.id, key);
    CommitTs cts = txmgr_.next_commit_ts();
    bool ok = store_.commit(txn.id, cts, txn.snapshot_ts);
    if (!ok) { store_.abort(txn.id); return false; }
    log_.append_del(key);
    return true;
}

std::vector<std::string> Engine::keys() {
    CommitTs snap = txmgr_.next_commit_ts() - 1;
    return store_.keys(snap, now_epoch());
}

Txn Engine::begin() {
    return txmgr_.begin();
}

bool Engine::commit(const Txn& txn) {
    // 1) copy staged writes before they are cleared by store_.commit()
    auto writes = store_.staged_writes(txn.id);

    // 2) get commit ts and try to install versions
    CommitTs cts = txmgr_.next_commit_ts();
    bool ok = store_.commit(txn.id, cts, txn.snapshot_ts);

    if (!ok) {
        // rollback staged changes from store (defensive)
        store_.abort(txn.id);
        return false;
    }

    // 3) durable WAL write for each staged write we copied
    //    (staged_writes were taken before commit, so we still have them)
    for (auto const& w : writes) {
        if (w.value.has_value()) {
            // expiry is already stored in w.expire_at (epoch seconds)
            log_.append_set(w.key, *w.value, w.expire_at);
        } else {
            log_.append_del(w.key);
        }
    }

    return true;
}


void Engine::abort(const Txn& txn) {
    store_.abort(txn.id);
}

void Engine::stage_set(const Txn& txn, const std::string& key, const std::string& value, int ttlSeconds) {
    std::time_t expiry = 0;
    if (ttlSeconds > 0) expiry = now_epoch() + ttlSeconds;
    // stage the write under txn.id (doesn't commit)
    store_.stage_set(txn.id, key, value, expiry);
}

void Engine::stage_del(const Txn& txn, const std::string& key) {
    store_.stage_del(txn.id, key);
}

std::optional<std::string> Engine::get(const Txn& txn, const std::string& key) {
    // 1) check staged writes for this txn
    auto staged = store_.staged_writes(txn.id);
    for (auto const &w : staged) {
        if (w.key == key) {
            // staged delete => nullopt
            if (!w.value.has_value()) return std::nullopt;
            return w.value;
        }
    }

    // 2) fallback to snapshot read at txn.snapshot_ts
    return store_.get(key, txn.snapshot_ts, now_epoch());
}
