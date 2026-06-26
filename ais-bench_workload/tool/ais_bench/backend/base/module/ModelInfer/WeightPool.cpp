#include "Base/ModelInfer/WeightPool.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

static const char* TAG_WEIGHT = "[weight_pool] ";

WeightPool& WeightPool::Instance() {
    static WeightPool instance;
    return instance;
}

UtilsResult::Result WeightPool::Acquire(const std::string& weightDir,
                                        aclmdlConfigHandle* handle,
                                        std::vector<std::string>& acquiredFiles) {
    acquiredFiles.clear();
    if (weightDir.empty() || handle == nullptr) {
        return UtilsResult::FAILED;
    }

    DIR* dir = opendir(weightDir.c_str());
    if (dir == nullptr) {
        return UtilsResult::FAILED;
    }

    std::vector<std::string> names;
    struct dirent* entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        names.push_back(name);
    }
    closedir(dir);
    std::sort(names.begin(), names.end());

    std::lock_guard<std::mutex> guard(mutex_);
    dirRefs_[weightDir] += 1;
    std::vector<std::string>& files = dirFiles_[weightDir];
    files.clear();

    size_t newCount = 0;
    size_t reusedCount = 0;
    for (const std::string& name : names) {
        std::string abspath = weightDir;
        if (abspath.back() != '/') {
            abspath += "/";
        }
        abspath += name;

        struct stat st;
        if (stat(abspath.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
            continue;
        }

        auto it = cache_.find(name);
        if (it != cache_.end()) {
            it->second.refCount += 1;
            aclError ret = aclmdlSetExternalWeightAddress(
                handle, name.c_str(), it->second.devPtr, it->second.size);
            if (ret != ACL_SUCCESS) {
                return UtilsResult::FAILED;
            }
            reusedCount += 1;
        } else {
            std::ifstream f(abspath, std::ios::binary | std::ios::ate);
            if (!f.is_open()) {
                return UtilsResult::FAILED;
            }
            std::streamsize rawSize = f.tellg();
            f.seekg(0, std::ios::beg);
            std::vector<uint8_t> data(rawSize);
            if (rawSize > 0 && !f.read(reinterpret_cast<char*>(data.data()), rawSize)) {
                return UtilsResult::FAILED;
            }

            size_t aligned = Align32(static_cast<size_t>(rawSize));
            void* devPtr = nullptr;
            aclError ret = aclrtMalloc(&devPtr, aligned, ACL_MEM_MALLOC_HUGE_FIRST);
            if (ret != ACL_SUCCESS) {
                std::cerr << TAG_WEIGHT << "aclrtMalloc failed for " << abspath
                          << " ret=" << ret << std::endl;
                return UtilsResult::FAILED;
            }
            ret = aclrtMemcpy(devPtr, aligned, data.data(),
                              static_cast<size_t>(rawSize), ACL_MEMCPY_HOST_TO_DEVICE);
            if (ret != ACL_SUCCESS) {
                aclrtFree(devPtr);
                return UtilsResult::FAILED;
            }

            WeightEntry e;
            e.devPtr = devPtr;
            e.size = aligned;
            e.rawSize = static_cast<size_t>(rawSize);
            e.refCount = 1;
            cache_[name] = e;

            ret = aclmdlSetExternalWeightAddress(handle, name.c_str(), devPtr, aligned);
            if (ret != ACL_SUCCESS) {
                std::cerr << TAG_WEIGHT << "aclmdlSetExternalWeightAddress failed for "
                          << name << " ret=" << ret << std::endl;
                return UtilsResult::FAILED;
            }
            newCount += 1;
        }

        files.push_back(name);
        acquiredFiles.push_back(name);
    }

    size_t total = acquiredFiles.size();
    size_t reusePct = total > 0 ? (reusedCount * 100 / total) : 0;
    size_t newPct = total > 0 ? (newCount * 100 / total) : 0;
    std::cout << TAG_WEIGHT << "dir=" << weightDir << " acquired "
              << total << " weights: " << reusedCount << " reused (" << reusePct
              << "%), " << newCount << " newly allocated (" << newPct << "%)"
              << std::endl;
    return UtilsResult::SUCCESS;
}

void WeightPool::Release(const std::string& weightDir) {
    if (weightDir.empty()) {
        return;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto refIt = dirRefs_.find(weightDir);
    if (refIt == dirRefs_.end()) {
        return;
    }
    refIt->second -= 1;

    auto filesIt = dirFiles_.find(weightDir);
    if (filesIt != dirFiles_.end()) {
        for (const std::string& name : filesIt->second) {
            auto it = cache_.find(name);
            if (it == cache_.end()) {
                continue;
            }
            if (it->second.refCount <= 1) {
                aclrtFree(it->second.devPtr);
                cache_.erase(it);
            } else {
                it->second.refCount -= 1;
            }
        }
        if (refIt->second <= 0) {
            dirFiles_.erase(filesIt);
        }
    }

    if (refIt->second <= 0) {
        dirRefs_.erase(refIt);
        std::cout << TAG_WEIGHT << "all references to " << weightDir
                  << " released" << std::endl;
    }
}

WeightPool::~WeightPool() {
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto& kv : cache_) {
        if (kv.second.devPtr != nullptr) {
            aclrtFree(kv.second.devPtr);
        }
    }
    cache_.clear();
}