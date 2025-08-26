#pragma once
#include <cstdint>
#include <atomic>
#include <optional>
#include <string>

namespace sapota {

using TxnId = uint64_t;
using CommitTs = uint64_t;

struct Txn {
    TxnId id;
    CommitTs snapshot_ts;
};

class MVCCStore;
class LogManager;

class TxnManager {
public:
    // Construct with references to MVCC store and LogManager (if needed)
    TxnManager();

    // begin returns a Txn object (id + snapshot)
    Txn begin();

    // get next commit timestamp (monotonic)
    CommitTs next_commit_ts();

private:
    std::atomic<TxnId> next_txn_id_{1};
    std::atomic<CommitTs> global_ts_{1};
};

} // namespace sapota
