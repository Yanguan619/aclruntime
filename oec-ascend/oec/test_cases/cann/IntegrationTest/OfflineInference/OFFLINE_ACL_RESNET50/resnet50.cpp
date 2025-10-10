#include "acl/acl.h"
#include <iostream>
#include <numeric>  // 添加此行以引入accumulate函数
#include <fstream>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
using namespace std;

// ---------------------- 全局变量定义 ----------------------
int32_t deviceId = 0;                // 计算设备ID
uint32_t modelId = 0;                 // 模型ID
size_t pictureDataSize = 0;           // 图片数据大小
void* pictureHostData = nullptr;      // 主机侧图片数据
void* pictureDeviceData = nullptr;    // 设备侧图片数据
aclmdlDataset* inputDataSet = nullptr;// 输入数据集
aclDataBuffer* inputDataBuffer = nullptr;
aclmdlDataset* outputDataSet = nullptr;// 输出数据集
aclDataBuffer* outputDataBuffer = nullptr;
aclmdlDesc* modelDesc = nullptr;      // 模型描述信息
size_t outputDataSize = 0;            // 输出数据大小
void* outputDeviceData = nullptr;     // 设备侧输出数据
void* outputHostData = nullptr;       // 主机侧输出数据

// ---------------------- 预期结果配置 ----------------------
const unsigned int EXPECTED_TOP1_INDEX = 162;  // 预期Top1类别索引（需根据模型数据集调整）
const double MIN_CONFIDENCE_THRESHOLD = 0.9;   // 最小置信度阈值（建议≥0.9）

// ---------------------- 函数声明 ----------------------
void InitResource();                // 资源初始化
void LoadModel(const char* modelPath);  // 加载模型
void LoadPicture(const char* picturePath); // 加载图片（主机+设备内存）
void Inference();                   // 执行推理
int PrintResultAndValidate();        // 打印结果并验证
void UnloadModel();                  // 卸载模型
void UnloadPicture();                // 释放图片相关资源
void DestroyResource();              // 释放全局资源

// ---------------------- 函数定义 ----------------------
// 1. 资源初始化（AscendCL初始化 + 指定计算设备）
void InitResource() {
    aclError ret = aclInit(nullptr); // 初始化AscendCL，使用默认配置
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] aclInit failed, error code: " << ret << endl;
        exit(1);
    }
    ret = aclrtSetDevice(deviceId); // 指定计算设备
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] aclrtSetDevice failed, error code: " << ret << endl;
        exit(1);
    }
}

// 2. 加载模型（.om文件）
void LoadModel(const char* modelPath) {
    aclError ret = aclmdlLoadFromFile(modelPath, &modelId);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to load model from " << modelPath << ", error code: " << ret << endl;
        exit(1);
    }
    cout << "[INFO] Model loaded successfully: " << modelPath << endl;
}

// 3. 读取图片到主机内存
void ReadPictureToHost(const char* picturePath) {
    ifstream binFile(picturePath, ios::binary);
    if (!binFile.is_open()) {
        cerr << "[ERROR] Failed to open picture file: " << picturePath << endl;
        exit(1);
    }
    // 获取文件大小并读取数据
    binFile.seekg(0, ios::end);
    pictureDataSize = binFile.tellg();
    binFile.seekg(0, ios::beg);
    
    aclError ret = aclrtMallocHost(&pictureHostData, pictureDataSize);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] aclrtMallocHost failed, error code: " << ret << endl;
        exit(1);
    }
    binFile.read((char*)pictureHostData, pictureDataSize);
    binFile.close();
    cout << "[INFO] Picture loaded to host memory: " << picturePath << endl;
}

// 4. 复制数据到设备内存
void CopyDataFromHostToDevice() {
    aclError ret = aclrtMalloc(&pictureDeviceData, pictureDataSize, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] aclrtMalloc failed, error code: " << ret << endl;
        exit(1);
    }
    ret = aclrtMemcpy(pictureDeviceData, pictureDataSize, pictureHostData, pictureDataSize, ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] aclrtMemcpy failed, error code: " << ret << endl;
        exit(1);
    }
    cout << "[INFO] Picture data copied to device memory" << endl;
}

// 5. 加载图片（组合函数）
void LoadPicture(const char* picturePath) {
    ReadPictureToHost(picturePath);
    CopyDataFromHostToDevice();
}

// 6. 创建模型输入数据结构
void CreateModelInput() {
    inputDataSet = aclmdlCreateDataset();
    inputDataBuffer = aclCreateDataBuffer(pictureDeviceData, pictureDataSize);
    aclError ret = aclmdlAddDatasetBuffer(inputDataSet, inputDataBuffer);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to create model input, error code: " << ret << endl;
        exit(1);
    }
}

// 7. 创建模型输出数据结构
void CreateModelOutput() {
    modelDesc = aclmdlCreateDesc();
    aclError ret = aclmdlGetDesc(modelDesc, modelId);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to get model description, error code: " << ret << endl;
        exit(1);
    }
    
    outputDataSet = aclmdlCreateDataset();
    outputDataSize = aclmdlGetOutputSizeByIndex(modelDesc, 0); // 获取第一个输出的大小
    
    ret = aclrtMalloc(&outputDeviceData, outputDataSize, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to allocate output memory, error code: " << ret << endl;
        exit(1);
    }
    outputDataBuffer = aclCreateDataBuffer(outputDeviceData, outputDataSize);
    ret = aclmdlAddDatasetBuffer(outputDataSet, outputDataBuffer);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to create model output, error code: " << ret << endl;
        exit(1);
    }
}

// 8. 执行推理
void Inference() {
    CreateModelInput();
    CreateModelOutput();
    aclError ret = aclmdlExecute(modelId, inputDataSet, outputDataSet);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Inference failed, error code: " << ret << endl;
        exit(1);
    }
}

// 9. 打印结果并验证
int PrintResultAndValidate() {
    // 复制输出数据到主机内存
    aclError ret = aclrtMallocHost(&outputHostData, outputDataSize);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to malloc host memory for output, error code: " << ret << endl;
        return 1;
    }
    ret = aclrtMemcpy(outputHostData, outputDataSize, outputDeviceData, outputDataSize, ACL_MEMCPY_DEVICE_TO_HOST);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to copy output data to host, error code: " << ret << endl;
        return 1;
    }
    
    // 解析输出数据（转换为float数组）
    float* outFloatData = reinterpret_cast<float*>(outputHostData);
    map<float, unsigned int, greater<float>> resultMap; // 按置信度降序排序
    for (unsigned int j = 0; j < outputDataSize / sizeof(float); ++j) {
        resultMap[outFloatData[j]] = j;
    }
    
    // 检查是否有推理结果
    if (resultMap.empty()) {
        cerr << "[ERROR] No inference results found" << endl;
        return 1;
    }
    
    // 提取Top1结果
    auto top1 = resultMap.begin();
    unsigned int top1Index = top1->second;
    double top1Score = top1->first;
    double top1Confidence = exp(top1Score) / accumulate(resultMap.begin(), resultMap.end(), 0.0,
        [](double sum, const pair<float, unsigned int>& item) { return sum + exp(item.first); });
    
    // 打印Top5结果
    cout << "\nTop 5 Inference Results:" << endl;
    int cnt = 0;
    for (auto it = resultMap.begin(); it != resultMap.end() && cnt < 5; ++it, ++cnt) {
        double prob = exp(it->first) / accumulate(resultMap.begin(), resultMap.end(), 0.0,
            [](double sum, const pair<float, unsigned int>& item) { return sum + exp(item.first); });
        cout << "Top " << cnt + 1 << ": Index[" << it->second << "] Confidence[" << fixed << prob << "]" << endl;
    }
    
    // 结果验证
    bool isSuccess = (top1Index == EXPECTED_TOP1_INDEX && top1Confidence >= MIN_CONFIDENCE_THRESHOLD);
    if (isSuccess) {
        cout << "\n[VALIDATION SUCCESS] Top1 matches expectations: Index[" << top1Index 
             << "] Confidence[" << fixed << top1Confidence << "]" << endl;
        return 0; // 验证通过，返回0
    } else {
        cerr << "\n[VALIDATION FAILED] Top1 does not match expectations:" << endl
             << "  Expected Index: " << EXPECTED_TOP1_INDEX << ", Confidence ≥ " << MIN_CONFIDENCE_THRESHOLD << endl
             << "  Actual Index: " << top1Index << ", Confidence: " << fixed << top1Confidence << endl;
        return 1; // 验证失败，返回1
    }
}

// 10. 卸载模型
void UnloadModel() {
    if (modelDesc != nullptr) {
        aclmdlDestroyDesc(modelDesc);
        modelDesc = nullptr;
    }
    aclError ret = aclmdlUnload(modelId);
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] Failed to unload model, error code: " << ret << endl;
    }
    cout << "[INFO] Model unloaded successfully" << endl;
}

// 11. 释放图片相关资源
void UnloadPicture() {
    if (pictureHostData != nullptr) {
        aclrtFreeHost(pictureHostData);
        pictureHostData = nullptr;
    }
    if (pictureDeviceData != nullptr) {
        aclrtFree(pictureDeviceData);
        pictureDeviceData = nullptr;
    }
    if (inputDataBuffer != nullptr) {
        aclDestroyDataBuffer(inputDataBuffer);
        inputDataBuffer = nullptr;
    }
    if (inputDataSet != nullptr) {
        aclmdlDestroyDataset(inputDataSet);
        inputDataSet = nullptr;
    }
    if (outputHostData != nullptr) {
        aclrtFreeHost(outputHostData);
        outputHostData = nullptr;
    }
    if (outputDeviceData != nullptr) {
        aclrtFree(outputDeviceData);
        outputDeviceData = nullptr;
    }
    if (outputDataBuffer != nullptr) {
        aclDestroyDataBuffer(outputDataBuffer);
        outputDataBuffer = nullptr;
    }
    if (outputDataSet != nullptr) {
        aclmdlDestroyDataset(outputDataSet);
        outputDataSet = nullptr;
    }
    cout << "[INFO] Picture resources unloaded successfully" << endl;
}

// 12. 释放全局资源
void DestroyResource() {
    aclError ret = aclrtResetDevice(deviceId); // 重置计算设备
    if (ret != ACL_SUCCESS) {
        cerr << "[ERROR] aclrtResetDevice failed, error code: " << ret << endl;
    }
    aclFinalize(); // 去初始化AscendCL
    cout << "[INFO] Global resources released successfully" << endl;
}

// ---------------------- 主函数 ----------------------
int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 3) {
        cerr << "[ERROR] Usage: " << argv[0] << " <base_path>"<<" <test_times>" << endl;
        cerr << "  Example: " << argv[0] << " /path/to/resources" <<" 1000" << endl;
        cerr << "  Model will be loaded from: <base_path>/model/resnet50.om" << endl;
        cerr << "  Picture will be loaded from: <base_path>/data/dog1_1024_683.bin" << endl;
        return 1;
    }
    
    // 构建模型和图片路径
    string basePath = argv[1];
    int test_times = atoi(argv[2]);
    string modelPath = basePath + "/model/resnet50.om";
    string picturePath = basePath + "/data/dog1_1024_683.bin";
    
    cout << "[INFO] Base path: " << basePath << endl;
    cout << "[INFO] Model path: " << modelPath << endl;
    cout << "[INFO] Picture path: " << picturePath << endl;
    
    // 1. 资源初始化
    InitResource();
    
    // 2. 加载模型
    LoadModel(modelPath.c_str());
    
    // 3. 加载测试图片
    LoadPicture(picturePath.c_str());
    double maxFps = 0;
    for(int j=0;j<3;++j){
        auto start = std::chrono::high_resolution_clock::now();
        for(int i =0; i < test_times; ++i){
            // 4. 执行推理
            Inference();
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double fps = static_cast<double>(test_times) / duration.count() * 1000000;
        maxFps = fps > maxFps? fps : maxFps;
        std::cout <<j<<". " << "FPS: " << fps << "\n" <<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // 5. 打印结果并验证
    int status = PrintResultAndValidate();
    
    
    std::string socName = aclrtGetSocName();

    std::map<std::string,double> baseLine;
    baseLine["Ascend310P3"] = 1632.7;
    baseLine["Ascend910B2"] = 1950.33;
    baseLine["Ascend910B3"] = 1941.3;
    baseLine["Ascend910B4"] = 1407.368;
    baseLine["Ascend910_9392"] = 2000.866;
    
    if (baseLine.find(socName) != baseLine.end()){
        double base = baseLine[socName];
        double delta = abs(maxFps - base) / base * 100;
        std::cout <<"soc: "<<socName<<" delta: " << delta << "%" <<std::endl;
        if(delta > 5){
            std::cout <<"ERROR: delta > 5%" <<std::endl;
            exit(1);
        }
    }else{
        std::cout <<"soc: "<<socName<<std::endl;
    }
    // 6. 卸载模型
    UnloadModel();
    
    // 7. 释放图片资源
    UnloadPicture();
    
    // 8. 释放全局资源
    DestroyResource();
    
    return status; // 0=成功，1=失败
}