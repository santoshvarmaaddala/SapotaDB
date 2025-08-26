#include "storage/mvcc.hpp"
#include <algorithm>
#include <ctime>
#include <mutex>

using namespace sapota;

MVCCStore::MVCCStore() = default;
MVCCStore::~MVCCStore() {
    std::unique_lock lk(mtx_);
    for (auto &p : map_) free_chain(p.second.head);
}

void MVCCStore::free_chain(Version* v) {
    while (v) { Version* n = v->next; delete v; v = n; }
}

const Version* MVCCStore::visible_version(const EntryChain& chain, CommitTs snapshot_ts, EpochSec now) {
    const Version* cur = chain.head;
    while (cur) {
        bool visible = (cur->begin_ts <= snapshot_ts) && (snapshot_ts < cur->end_ts);
        bool alive = (cur->expire_at == 0) || (now < cur->expire_at);
        if (visible && alive) return cur;
        cur = cur->next;
    }
    return nullptr;
}

std::optional<std::string> MVCCStore::get(const std::string& key, CommitTs snapshot_ts, EpochSec now) const {
    std::shared_lock lk(mtx_);
    auto it = map_.find(key);
    if (it == map_.end()) return std::nullopt;
    const Version* v = visible_version(it->second, snapshot_ts, now);
    if (!v) return std::nullopt;
    if (v->value.empty()) return std::nullopt; // tombstone
    return v->value;
}

void MVCCStore::stage_set(TxnId txn, const std::string& key, const std::string& value, EpochSec expire_at) {
    std::unique_lock lk(mtx_);
    staged_[txn].push_back({key, value, expire_at});
}

void MVCCStore::stage_del(TxnId txn, const std::string& key) {
    std::unique_lock lk(mtx_);
    staged_[txn].push_back({key, std::nullopt, 0});
}

std::vector<MVCCStore::WriteOp> MVCCStore::staged_writes(TxnId txn) const {
    std::shared_lock lk(mtx_);
    std::vector<WriteOp> out;
    auto it = staged_.find(txn);
    if (it == staged_.end()) return out;
    out.reserve(it->second.size());
    for (auto const& pw : it->second) out.push_back(WriteOp{pw.key, pw.value, pw.expire_at});
    return out;
}

bool MVCCStore::commit(TxnId txn, CommitTs commit_ts, CommitTs read_snapshot_ts) {
    std::unique_lock lk(mtx_);
    auto it = staged_.find(txn);
    if (it == staged_.end()) return true;

    // conflict detection: if head.begin_ts > read_snapshot_ts => someone committed new version after our snapshot
    for (auto const& pw : it->second) {
        auto mit = map_.find(pw.key);
        if (mit == map_.end()) continue;
        Version* head = mit->second.head;
        if (head && head->begin_ts > read_snapshot_ts) {
            return false;
        }
    }

    // install new versions
    for (auto const& pw : it->second) {
        EntryChain& chain = map_[pw.key];
        if (chain.head) chain.head->end_ts = commit_ts;
        Version* v = new Version();
        v->value = pw.value.has_value() ? *pw.value : std::string();
        v->begin_ts = commit_ts;
        v->end_ts = MVCC_INF_TS;
        v->expire_at = pw.expire_at;
        v->next = chain.head;
        chain.head = v;
    }

    staged_.erase(it);
    return true;
}

void MVCCStore::abort(TxnId txn) {
    std::unique_lock lk(mtx_);
    staged_.erase(txn);
}

std::vector<std::string> MVCCStore::keys(CommitTs snapshot_ts, EpochSec now) const {
    std::vector<std::string> out;
    std::shared_lock lk(mtx_);
    out.reserve(map_.size());
    for (auto const& kv : map_) {
        const Version* v = visible_version(kv.second, snapshot_ts, now);
        if (!v) continue;
        if (v->value.empty()) continue;
        out.push_back(kv.first);
    }
    return out;
}

void MVCCStore::vacuum_expired(EpochSec now) {
    std::unique_lock lk(mtx_);
    for (auto &p : map_) {
        Version* prev = nullptr;
        Version* cur = p.second.head;
        while (cur) {
            bool expired = (cur->expire_at != 0) && (now >= cur->expire_at);
            if (expired) {
                Version* del = cur;
                if (prev) prev->next = cur->next;
                else p.second.head = cur->next;
                cur = cur->next;
                delete del;
                continue;
            }
            prev = cur;
            cur = cur->next;
        }
    }
}

void MVCCStore::gc_old_versions(CommitTs safe_ts) {
    std::unique_lock lk(mtx_);
    for (auto &p : map_) {
        Version* cur = p.second.head;
        if (!cur) continue;
        Version* prev = cur;
        cur = cur->next;
        while (cur) {
            if (cur->end_ts <= safe_ts) {
                prev->next = cur->next;
                delete cur;
                cur = prev->next;
            } else {
                prev = cur;
                cur = cur->next;
            }
        }
    }
}
