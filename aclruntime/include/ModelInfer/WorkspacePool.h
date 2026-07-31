#ifndef ACLRUNTIME_INCLUDE_MODELINFER_WORKSPACEPOOL_H_
#define ACLRUNTIME_INCLUDE_MODELINFER_WORKSPACEPOOL_H_

#include <map>
#include <mutex>
#include <string>

#include "ModelInfer/utils.h"
#include "acl/acl.h"

class WorkspacePool {
public:
    struct WorkspaceEntry {
        void* devPtr = nullptr;
        size_t size = 0;
        int refCount = 0;
    };

    static WorkspacePool& Instance();

    UtilsResult::Result Acquire(const std::string& group, size_t requiredSize,
                                void*& workspaceAddr, size_t& allocatedSize);

    void Release(const std::string& group);

private:
    WorkspacePool() = default;
    ~WorkspacePool();
    WorkspacePool(const WorkspacePool&) = delete;
    WorkspacePool& operator=(const WorkspacePool&) = delete;

    std::mutex mutex_;
    std::map<std::string, WorkspaceEntry> cache_;
};

#endif
