#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <shared_mutex>
#include <cstdint>

namespace sapota {

using TxnId = uint64_t;
using CommitTs = uint64_t;
using EpochSec = int64_t;
static constexpr CommitTs MVCC_INF_TS = std::numeric_limits<CommitTs>::max();

struct Version {
    std::string value;
    CommitTs begin_ts{0};
    CommitTs end_ts{MVCC_INF_TS};
    EpochSec expire_at{0}; // 0 = none
    Version* next{nullptr};
};

struct EntryChain {
    Version* head{nullptr};
};

class MVCCStore {
public:
    MVCCStore();
    ~MVCCStore();

    // read as-of snapshot_ts; returns empty optional if not visible or tombstone
    std::optional<std::string> get(const std::string& key, CommitTs snapshot_ts, EpochSec now) const;

    // stage writes (kept per-txn until commit)
    void stage_set(TxnId txn, const std::string& key, const std::string& value, EpochSec expire_at);
    void stage_del(TxnId txn, const std::string& key);

    // retrieve staged operations for a txn (copy)
    struct WriteOp {
        std::string key;
        std::optional<std::string> value; // nullopt => delete
        EpochSec expire_at{0};
    };
    std::vector<WriteOp> staged_writes(TxnId txn) const;

    // commit installed with commit_ts and read_snapshot for conflict detection
    bool commit(TxnId txn, CommitTs commit_ts, CommitTs read_snapshot_ts);

    // abort: drop staged
    void abort(TxnId txn);

    // keys visible at snapshot_ts
    std::vector<std::string> keys(CommitTs snapshot_ts, EpochSec now) const;

    // maintenance
    void vacuum_expired(EpochSec now);
    void gc_old_versions(CommitTs safe_ts);

private:
    static const Version* visible_version(const EntryChain& chain, CommitTs snapshot_ts, EpochSec now);

    mutable std::shared_mutex mtx_;
    std::unordered_map<std::string, EntryChain> map_;
    struct PendingWrite { std::string key; std::optional<std::string> value; EpochSec expire_at{0}; };
    std::unordered_map<TxnId, std::vector<PendingWrite>> staged_;
    static void free_chain(Version* v);
};

} // namespace sapota
