#ifndef WEIGHT_POOL_H_
#define WEIGHT_POOL_H_

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "acl/acl.h"
#include "ModelInfer/utils.h"

// Process-wide cache of device weight buffers, keyed by weight file
// *basename*. External weights are registered on the OM config handle by
// the basename ACL embeds in the model (e.g. "weight_<sha256>"), and the
// naming scheme is a content hash, so identical basenames imply identical
// bytes. Keying by basename therefore lets prefill and decode OM files —
// which live in separate directories but share the same decoder weights —
// reuse the same device allocations and pay weight memory only once.
class WeightPool {
public:
    struct WeightEntry {
        void* devPtr = nullptr;
        size_t size = 0;        // aligned size (32-byte aligned)
        size_t rawSize = 0;     // original file size
        int refCount = 0;
    };

    static WeightPool& Instance();

    // List every regular file in `weightDir` (sorted), ensure each has a
    // device buffer in the cache (allocated on the currently-set acl
    // context), then register every (fileName, devPtr, size) on `handle`
    // via aclmdlSetExternalWeightAddress. Returns SUCCESS on success and
    // fills `acquiredFiles` with the file names registered. Reuses cached
    // buffers when a file was already loaded by another session and only
    // bumps the refcount in that case.
    UtilsResult::Result Acquire(const std::string& weightDir,
                                aclmdlConfigHandle* handle,
                                std::vector<std::string>& acquiredFiles);

    // Drop one reference to `weightDir`. For each file tracked for that
    // dir the per-entry refcount is decremented; entries reaching zero are
    // freed with aclrtFree (the caller must have the right context set).
    void Release(const std::string& weightDir);

private:
    WeightPool() = default;
    ~WeightPool();
    WeightPool(const WeightPool&) = delete;
    WeightPool& operator=(const WeightPool&) = delete;

    static size_t Align32(size_t size) { return (size + 31UL) & ~31UL; }

    std::mutex mutex_;
    // weight file basename -> buffer entry
    std::map<std::string, WeightEntry> cache_;
    // weightDir -> list of weight basenames registered for that dir
    std::map<std::string, std::vector<std::string>> dirFiles_;
    // weightDir -> number of outstanding Acquire references
    std::map<std::string, int> dirRefs_;
};

#endif  // WEIGHT_POOL_H_
