/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/time.h>
#include "utils.h"
#include "acl/acl.h"

using namespace std;
namespace {
bool g_isDevice = true;
const uint16_t OPEN_FILE_MODE = 640;
const uint32_t MAX_OPEN_FILES_SIZE = 64 * 1024 * 1024;
const mode_t OPNE_OR_CREATE_MODE = O_EXCL | O_CREAT;
const mode_t CREATE_FILE_MODE = S_IRUSR | S_IWUSR | S_IRGRP;
}

void Utils::SplitString(std::string& s, std::vector<std::string>& v, char c)
{
    std::string::size_type pos1;
    std::string::size_type pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
        std::string s1 = s.substr(pos1, pos2 - pos1);
        size_t n = s1.find_last_not_of(" \r\n\t");
        if (n != string::npos) {
            s1.erase(n + 1, s.size() - n);
        }
        n = s1.find_first_not_of(" \r\n\t");
        if (n != string::npos) {
            s1.erase(0, n);
        }
        v.push_back(s1);
        pos1 = pos2 + 1;
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length()) {
        std::string s1 = s.substr(pos1);
        size_t n = s1.find_last_not_of(" \r\n\t");
        if (n != string::npos) {
            s1.erase(n + 1, s.size() - n);
        }
        n = s1.find_first_not_of(" \r\n\t");
        if (n != string::npos) {
            s1.erase(0, n);
        }
        v.push_back(s1);
    }
}

int Utils::str2num(char* str)
{
    int n = 0;
    int flag = 0;
    const int decimal = 10;
    while (*str >= '0' && *str <= '9') {
        if (n > INT_MAX / 10 || (n == INT_MAX / 10 && (*str - '0') > INT_MAX % 10)) {
            throw std::runtime_error("string format int overflowed");
        }
        if (n < INT_MIN / 10 || (n == INT_MIN / 10 && (*str - '0') > -(INT_MIN % 10))) {
            throw std::runtime_error("string format int underflowed");
        }
        n = n * decimal + (*str - '0');
        str++;
    }
    if (flag == 1) {
        n = -n;
    }
    return n;
}

std::string Utils::modelName(string& s)
{
    string::size_type position1;
    string::size_type position2;
    position1 = s.find_last_of("/");
    if (position1 == s.npos) {
        position1 = 0;
    } else {
        position1 = position1 + 1;
    }
    position2 = s.find_last_of(".");
    if (position1 > position2) {
        throw std::runtime_error("illegal model name string.");
    }
    std::string modelName = s.substr(position1, position2 - position1);
    return modelName;
}

std::string Utils::TimeLine()
{
    time_t currentTime = time(NULL);
    char chCurrentTime[64];
    strftime(chCurrentTime, sizeof(chCurrentTime), "%Y%m%d_%H%M%S", localtime(&currentTime));
    std::string stCurrentTime = chCurrentTime;
    return stCurrentTime;
}

std::string Utils::printCurrentTime()
{
    struct timeval tv;
    struct timezone tz;
    struct tm* p = nullptr;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);
    if (p == nullptr) {
        std::runtime_error("get local time failed");
    }
    std::string pi = std::to_string(p->tm_year + 1900) + std::to_string(p->tm_mon + 1) + std::to_string(p->tm_mday) \
             + "_" + std::to_string(p->tm_hour) + "_" + std::to_string(p->tm_min) + "_" + \
             std::to_string(p->tm_sec) + "_" + std::to_string(tv.tv_usec);
    return pi;
}

double Utils::printDiffTime(time_t begin, time_t end)
{
    double diffT = difftime(begin, end);
    const double sec_to_msec = 1000;
    PROMPT_MSG("The inference time is: %f millisecond\n", sec_to_msec * diffT);
    return diffT * sec_to_msec;
}

void Utils::SplitStringSimple(string str, vector<string> &out, char split1, char split2, char split3)
{
    istringstream block(str);
    string cell;
    string cell1;
    string cell2;
    vector<string> split1_out;
    vector<string> split2_out;
    while (getline(block, cell, split1)) {
        split1_out.push_back(cell);
    }

    // find the last split2 because split2 only once
    for (auto var : split1_out) {
        size_t pos = var.rfind(split2);
        if (pos != var.npos) {
            split2_out.push_back(var.substr(pos + 1, var.size()-pos-1));
        }
    }

    for (size_t i = 0; i < split2_out.size(); ++i) {
        istringstream block_tmp1(split2_out[i]);
        while (getline(block_tmp1, cell2, split3)) {
            out.push_back(cell2);
        }
    }
}

void Utils::SplitStringWithSemicolonsAndColons(string str, vector<string> &out, char split1, char split2)
{
    istringstream block(str);
    string cell;
    string cell1;
    vector<string> split1_out;

    while (getline(block, cell, split1)) {
        split1_out.push_back(cell);
    }
    for (size_t i = 0; i < split1_out.size(); ++i) {
        istringstream block_tmp(split1_out[i]);
        int index = 0;
        while (getline(block_tmp, cell1, split2)) {
            if (index == 1) {
                out.push_back(cell1);
            }
            index += 1;
        }
    }
}

void Utils::SplitStringWithPunctuation(string str, vector<string> &out, char split)
{
    istringstream block(str);
    string cell;
    while (getline(block, cell, split)) {
        out.push_back(cell);
    }
}

int Utils::ToInt(string &str)
{
    return atoi(str.c_str());
}

Result Utils::SplitStingGetNameDimsMulMap(std::vector<std::string> in_dym_shape_str,
    std::map<string, int64_t> &out_namedimsmul_map)
{
    string name;
    string shape_str;

    for (size_t i = 0; i < in_dym_shape_str.size(); ++i) {
        size_t pos = in_dym_shape_str[i].rfind(':');
        if (pos == in_dym_shape_str[i].npos) {
            ERROR_LOG("find no : split i:%zu str:%s\n", i, in_dym_shape_str[i].c_str());
            return FAILED;
        }
        name = in_dym_shape_str[i].substr(0, pos);
        shape_str = in_dym_shape_str[i].substr(pos + 1, in_dym_shape_str[i].size()-pos-1);

        vector<string> shape_tmp;
        Utils::SplitStringWithPunctuation(shape_str, shape_tmp, ',');
        int64_t DimsMul = 1;
        for (size_t j = 0; j < shape_tmp.size(); ++j) {
	        DimsMul = DimsMul * atoi(shape_tmp[j].c_str());
        }
        out_namedimsmul_map[name] = DimsMul;
    }
    return SUCCESS;
}

Result Utils::ReadBinFileToMemory(const std::string fileName, char *ptr, const size_t size, size_t &offset)
{
    std::ifstream binFile;
    if (!File::OpenFile(fileName, binFile, std::ifstream::binary)) {
        ERROR_LOG("read bin file to memory: open file %s failed.", fileName.c_str());
        return FAILED;
    }

    binFile.seekg(0, binFile.end);
    uint64_t binFileBufferLen = binFile.tellg();
    if (binFileBufferLen == 0) {
        ERROR_LOG("bin file is empty, filename is %s", fileName.c_str());
        binFile.close();
        return FAILED;
    }
    if (offset + binFileBufferLen > size) {
        ERROR_LOG("offset:%zu filesize:%zu > size:%zu invalid", offset, binFileBufferLen, size);
        return FAILED;
    }

    binFile.seekg(0, binFile.beg);

    if (ptr == nullptr) {
        ERROR_LOG("ptr is nullptr");
        binFile.close();
        return FAILED;
    }
    DEBUG_LOG("Readbin file:%s offset:%zu len:%zu\n", fileName.c_str(), offset, binFileBufferLen);
    binFile.read(static_cast<char*>(ptr + offset), binFileBufferLen);
    binFile.close();
    offset += binFileBufferLen;
    return SUCCESS;
}

Result Utils::FillFileContentToMemory(const std::string file, char* ptr, const size_t size, size_t &offset)
{
    auto ret = Utils::ReadBinFileToMemory(file, ptr, size, offset);
    if (ret != SUCCESS) {
        ERROR_LOG("read bin file to memory failed ret:%d", ret);
        return ret;
    }
    return SUCCESS;
}

std::string Utils::MergeStr(std::vector<std::string>& list, const std::string& delimiter)
{
    auto res = std::accumulate(list.begin(), list.end(), std::string(),
    [=](const std::string& a, const std::string& b) -> std::string {
        return a + (a.length() > 0 ? delimiter : "") + b; });
    return res;
}

std::string Utils::GetPrefix(const std::string& outputDir, std::string filePath, const std::string& removeTail)
{
    std::stringstream inStream(filePath);
    std::string fileName {};
    while (inStream.good()) {
        std::string subStr = "";
        getline(inStream, subStr, '/');
        if (subStr == "") {
            continue;
        }
        fileName = subStr;
    }

    // remove tail ".npy" or ".bin"
    if (fileName.size() >= removeTail.size()
        && fileName.compare(fileName.size() - removeTail.size(), removeTail.size(), removeTail) == 0) {
        fileName.erase(fileName.size() - removeTail.size());
    }
    return outputDir + "/" + fileName + "_";
}

std::string Utils::RemoveSlash(const std::string& name)
{
    std::string res;
    for (auto &elem: name) {
        if (elem != '/') {
            res.push_back(elem);
        }
    }
    return res;
}

std::string Utils::CreateDynamicShapeDims(const std::string& name, std::vector<size_t>& shapes)
{
    std::vector<std::string> shapeStr {};
    for (auto &shape : shapes) {
        shapeStr.emplace_back(std::to_string(shape));
    }
    auto res = Utils::MergeStr(shapeStr, ",");
    return name + ":" + res;
}

Result Utils::TensorToNumpy(const std::string& outputFileName, Base::TensorBase& output)
{
    auto shapeTmp = output.GetShape();
    std::vector<size_t> shape {shapeTmp.begin(), shapeTmp.end()};
    // std::string typeName = DATA_TYPE_TO_STRING_MAP.at(output.GetDataType());
    // std::stringstream stype(typeName);
    // cnpy::NpySave(outputFileName, (stype*)output.GetBuffer(), shape);
    if (output.GetDataType() == Base::TENSOR_DTYPE_FLOAT32) {
        cnpy::NpySave(outputFileName, (float*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_FLOAT16) {
        cnpy::NpySave(outputFileName, (aclFloat16*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT8) {
        cnpy::NpySave(outputFileName, (int8_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT32) {
        cnpy::NpySave(outputFileName, (int32_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT8) {
        cnpy::NpySave(outputFileName, (uint8_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT16) {
        cnpy::NpySave(outputFileName, (int16_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT16) {
        cnpy::NpySave(outputFileName, (uint16_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT32) {
        cnpy::NpySave(outputFileName, (uint32_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT64) {
        cnpy::NpySave(outputFileName, (int64_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT64) {
        cnpy::NpySave(outputFileName, (uint64_t*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_DOUBLE64) {
        cnpy::NpySave(outputFileName, (double*)output.GetBuffer(), shape);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_BOOL) {
        cnpy::NpySave(outputFileName, (bool*)output.GetBuffer(), shape);
    } else {
        ERROR_LOG("Tensor to numpy: output data type unrecognized.");
        return FAILED;
    }
    return SUCCESS;
}

Result Utils::TensorToBin(const std::string& outputFileName, Base::TensorBase& output)
{
    if (access(outputFileName.c_str(), F_OK) == 0 && remove(outputFileName.c_str()) != 0) {
        ERROR_LOG("Tensor to bin: existing file %s cannot be removed", outputFileName.c_str());
        return FAILED;
    }
    std::ofstream outfile;
    if (!File::OpenFile(outputFileName, outfile, std::ios::out | std::ios::binary)) {
        ERROR_LOG("Tensor to bin: open file %s failed.", outputFileName.c_str());
        return FAILED;
    }

    outfile.write(reinterpret_cast<const char*>(output.GetBuffer()), output.GetByteSize());
    outfile.close();

    return SUCCESS;
}

template <typename T>
static void SaveTxt(std::ofstream& outFile, const T* p, size_t size, size_t rowCount)
{
    if (p == nullptr) {
        throw runtime_error("SaveTxt: data pointer is null");
    }
    std::vector<T> nums (p, p + size);
    size_t count = 0;
    for (auto num: nums) {
        outFile << num << " ";
        count++;
        if (count == rowCount) {
            outFile << std::endl;
            count = 0;
        }
    }
}

Result Utils::TensorToTxt(const std::string& outputFileName, Base::TensorBase& output)
{
    if (access(outputFileName.c_str(), F_OK) == 0 && remove(outputFileName.c_str()) != 0) {
        ERROR_LOG("Tensor to txt: existing file %s cannot be removed", outputFileName.c_str());
        return FAILED;
    }
    std::ofstream outFile;
    if (!File::OpenFile(outputFileName, outFile)) {
        ERROR_LOG("Tensor to txt: open file %s failed.", outputFileName.c_str());
        return FAILED;
    }
    size_t size = output.GetSize();
    size_t rowCount = output.GetShape().back();

    if (output.GetDataType() == Base::TENSOR_DTYPE_FLOAT32) {
        SaveTxt(outFile, (float*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_FLOAT16) {
        SaveTxt(outFile, (aclFloat16*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT8) {
        SaveTxt(outFile, (int8_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT32) {
        SaveTxt(outFile, (int32_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT8) {
        SaveTxt(outFile, (uint8_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT16) {
        SaveTxt(outFile, (int16_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT16) {
        SaveTxt(outFile, (uint16_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT32) {
        SaveTxt(outFile, (uint32_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_INT64) {
        SaveTxt(outFile, (int64_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_UINT64) {
        SaveTxt(outFile, (uint64_t*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_DOUBLE64) {
        SaveTxt(outFile, (double*)output.GetBuffer(), size, rowCount);
    } else if (output.GetDataType() == Base::TENSOR_DTYPE_BOOL) {
        SaveTxt(outFile, (bool*)output.GetBuffer(), size, rowCount);
    } else {
        ERROR_LOG("Tensor to bin: output data type unrecognized.");
        return FAILED;
    }
    return SUCCESS;
}

bool Utils::TailContain(const std::string& str, const std::string& tail)
{
    if (str.length() >= tail.length() && str.compare(str.length() - tail.length(), tail.length(), tail) == 0) {
        return true;
    }
    return false;
}