// 用 aclmdlSetExternalWeightAddress 让两个 OM 模型共享同一块 Device 权重内存。
// 这里把同一个 OM 加载两次 (两个 modelId), 演示它们复用同一份 Device 权重。
//
// 前置: OM 用 ATC --external_weight=1 编译, 权重剥离到同级 weight/ 目录的
// weight_<hash> 文件。
// 编译: g++ -std=c++17 weight_sharing_sample.cpp \
//        -I/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/include \
//        -L/usr/local/Ascend/ascend-toolkit/latest/lib64 -lascendcl -o
//        weight_sharing_sample
// 运行: export LD_LIBRARY_PATH=/usr/local/Ascend/driver/lib64:/usr/lib64:\
//        /usr/local/Ascend/ascend-toolkit/latest/lib64:$LD_LIBRARY_PATH
//       ./weight_sharing_sample [om_path]
#include <acl/acl.h>
#include <dirent.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static void Check(int ret, const std::string& msg) {
    if (ret != ACL_SUCCESS) {
        std::cerr << "[ERROR] " << msg << " ret=" << ret << std::endl;
        exit(ret);
    }
}

// 读 weight_dir 下所有权重文件上 Device, 只一份。返回 {文件名: (devPtr,
// 对齐后大小)}。
static std::vector<std::pair<std::string, std::pair<void*, size_t>>>
LoadSharedWeights(const std::string& weightDir) {
    std::vector<std::pair<std::string, std::pair<void*, size_t>>> weights;
    DIR* dir = opendir(weightDir.c_str());
    Check(dir == nullptr ? -1 : 0, "opendir " + weightDir);
    std::vector<std::string> names;
    for (struct dirent* e; (e = readdir(dir)) != nullptr;) {
        if (e->d_name[0] != '.') names.emplace_back(e->d_name);
    }
    closedir(dir);
    std::sort(names.begin(), names.end());

    for (const auto& name : names) {
        std::string p = weightDir + "/" + name;
        std::ifstream f(p, std::ios::binary | std::ios::ate);
        Check(!f.is_open() ? -1 : 0, "open " + p);
        size_t raw = f.tellg();
        f.seekg(0);
        std::vector<char> buf(raw);
        if (raw) f.read(buf.data(), raw);

        size_t aligned = (raw + 31) & ~size_t(31);  // 需 32 字节对齐
        void* dev = nullptr;
        Check(aclrtMalloc(&dev, aligned, ACL_MEM_MALLOC_HUGE_FIRST),
              "aclrtMalloc " + name);
        Check(aclrtMemcpy(dev, aligned, buf.data(), raw,
                          ACL_MEMCPY_HOST_TO_DEVICE),
              "aclrtMemcpy " + name);
        weights.emplace_back(name, std::make_pair(dev, aligned));
        std::cout << "[weight] " << name << ": " << raw << " bytes -> device "
                  << dev << std::endl;
    }
    return weights;
}

// 用 aclmdlLoadWithConfig 加载一个 OM, 把共享权重登记到它的配置对象。
static uint32_t LoadModel(
    const std::string& omPath,
    const std::vector<std::pair<std::string, std::pair<void*, size_t>>>& w) {
    aclmdlConfigHandle* h = aclmdlCreateConfigHandle();
    Check(h == nullptr ? -1 : 0, "aclmdlCreateConfigHandle");

    // 把每个外置权重的 Device 地址按文件名登记 (两个模型传同一个 devPtr ->
    // 共享)
    for (const auto& [name, devsz] : w) {
        Check(aclmdlSetExternalWeightAddress(h, name.c_str(), devsz.first,
                                             devsz.second),
              "aclmdlSetExternalWeightAddress " + name);
    }

    // 加载方式 = 从文件加载
    size_t loadType = ACL_MDL_LOAD_FROM_FILE;
    Check(aclmdlSetConfigOpt(h, ACL_MDL_LOAD_TYPE_SIZET, &loadType,
                             sizeof(loadType)),
          "set ACL_MDL_LOAD_TYPE_SIZET");
    // PATH_PTR 属性本身是指针, 按文档传入该指针的地址
    const char* path = omPath.c_str();
    Check(aclmdlSetConfigOpt(h, ACL_MDL_PATH_PTR, &path, sizeof(path)),
          "set ACL_MDL_PATH_PTR");

    uint32_t modelId = 0;
    Check(aclmdlLoadWithConfig(h, &modelId), "aclmdlLoadWithConfig " + omPath);
    Check(aclmdlDestroyConfigHandle(h), "aclmdlDestroyConfigHandle");
    std::cout << "[model] loaded " << omPath << " -> modelId=" << modelId
              << std::endl;
    return modelId;
}

int main(int argc, char** argv) {
    std::string om =
        argc > 1 ? argv[1] : "/data/workspace/qwen2onnx/resnet50.om";
    std::string wdir = om.substr(0, om.find_last_of('/')) + "/weight";

    Check(aclInit(nullptr), "aclInit");
    Check(aclrtSetDevice(0), "aclrtSetDevice");

    // 1. 权重上 Device, 只一份
    auto weights = LoadSharedWeights(wdir);
    std::cout << "[weight] " << weights.size()
              << " buffers on device (single copy)\n";

    // 2. 同一个 OM 加载两次, 共享同一份权重
    uint32_t a = LoadModel(om, weights);
    uint32_t b = LoadModel(om, weights);

    // 3. 清理: 卸载模型 -> 释放共享权重 -> finalize
    aclmdlUnload(a);
    aclmdlUnload(b);
    for (const auto& [_, devsz] : weights) aclrtFree(devsz.first);
    aclFinalize();
    std::cout << "done, two models shared one weight buffer, now released\n";
    return 0;
}
