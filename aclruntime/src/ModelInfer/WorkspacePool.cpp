#include "ModelInfer/WorkspacePool.h"

#include "Log.h"

static const char* TAG_WS = "workspace_pool";

static std::string FormatSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double val = static_cast<double>(bytes);
    while (val >= 1024.0 && unit < 3) {
        val /= 1024.0;
        unit++;
    }
    char buf[32];
    if (unit == 0 || val >= 100.0) {
        snprintf(buf, sizeof(buf), "%.1f %s", val, units[unit]);
    } else if (val >= 10.0) {
        snprintf(buf, sizeof(buf), "%.2f %s", val, units[unit]);
    } else {
        snprintf(buf, sizeof(buf), "%.3f %s", val, units[unit]);
    }
    return buf;
}

WorkspacePool& WorkspacePool::Instance() {
    static WorkspacePool instance;
    return instance;
}

UtilsResult::Result WorkspacePool::Acquire(const std::string& group,
                                           size_t requiredSize,
                                           void*& workspaceAddr,
                                           size_t& allocatedSize) {
    if (group.empty() || requiredSize == 0) {
        return UtilsResult::FAILED;
    }

    std::lock_guard<std::mutex> guard(mutex_);

    auto it = cache_.find(group);
    if (it != cache_.end()) {
        if (it->second.size >= requiredSize) {
            it->second.refCount += 1;
            workspaceAddr = it->second.devPtr;
            allocatedSize = it->second.size;
            INFO_LOG("[%s] reuse workspace for group '%s' (size=%s, ref=%d)",
                     TAG_WS, group.c_str(),
                     FormatSize(it->second.size).c_str(),
                     it->second.refCount);
            return UtilsResult::SUCCESS;
        }
        // Do NOT reallocate — previously loaded models already hold a
        // reference to the old devPtr. Replacing it would create a
        // dangling pointer for those models. The caller must load models
        // in descending workspace size order or use separate groups.
        ERROR_LOG("[%s] workspace for group '%s' (%s) is too small for "
                  "requested %s. Load models in descending workspace size "
                  "order, or use a separate group.",
                  TAG_WS, group.c_str(),
                  FormatSize(it->second.size).c_str(),
                  FormatSize(requiredSize).c_str());
        return UtilsResult::FAILED;
    }

    void* devPtr = nullptr;
    aclError ret = MemCheckedCall("aclrtMallocAlign32", aclrtMallocAlign32,
                                  &devPtr, requiredSize,
                                  ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("[%s] aclrtMallocAlign32 failed for group '%s', size=%s",
                  TAG_WS, group.c_str(),
                  FormatSize(requiredSize).c_str());
        return UtilsResult::FAILED;
    }

    WorkspaceEntry e;
    e.devPtr = devPtr;
    e.size = requiredSize;
    e.refCount = 1;
    cache_[group] = e;

    workspaceAddr = devPtr;
    allocatedSize = requiredSize;

    INFO_LOG("[%s] allocated workspace for group '%s' (size=%s)", TAG_WS,
             group.c_str(), FormatSize(requiredSize).c_str());
    return UtilsResult::SUCCESS;
}

void WorkspacePool::Release(const std::string& group) {
    if (group.empty()) {
        return;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto it = cache_.find(group);
    if (it == cache_.end()) {
        return;
    }
    it->second.refCount -= 1;
    INFO_LOG("[%s] release workspace for group '%s' (ref=%d)", TAG_WS,
             group.c_str(), it->second.refCount);

    if (it->second.refCount <= 0) {
        if (it->second.devPtr != nullptr) {
            aclrtFree(it->second.devPtr);
        }
        cache_.erase(it);
        INFO_LOG("[%s] freed workspace for group '%s'", TAG_WS,
                 group.c_str());
    }
}

WorkspacePool::~WorkspacePool() {
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto& kv : cache_) {
        if (kv.second.devPtr != nullptr) {
            aclrtFree(kv.second.devPtr);
        }
    }
    cache_.clear();
}
