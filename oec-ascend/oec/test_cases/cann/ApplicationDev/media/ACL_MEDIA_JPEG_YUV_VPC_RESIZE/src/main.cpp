/**
* @file main.cpp
*
* Copyright (C) 2020. Huawei Technologies Co., Ltd. All rights reserved.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/
#include <iostream>
#include <stdlib.h>
#include <string>
#include "sample_process.h"
#include "utils.h"

using namespace std;
string data_path ,output_path;
int main(int argc,const char *argv[])
{
    INFO_LOG("./main param, param represents a vpc feature and must be set");
    if (argc != 4) {
        ERROR_LOG("input param not be set");
        return FAILED;
    }
    data_path = string(argv[2]);
    output_path = string(argv[3]);
    string result_path_str = output_path + "/result";
    Result ret = Utils::CheckAndCreateFolder(result_path_str.c_str());
    if (ret != SUCCESS) {
        ERROR_LOG("mkdir out folder error, dir = %s.", result_path_str.c_str());
        return FAILED;
    }

    SampleProcess sampleProcess;
    ret = sampleProcess.InitResource();
    if (ret != SUCCESS) {
        ERROR_LOG("sample init resource failed");
        return FAILED;
    }

    DvppType dvppType = static_cast<DvppType>(atoi(argv[1]));
    if (dvppType == JPEG_ENCODE) {
        ret = sampleProcess.JpegeProcess(dvppType);
        if (ret != SUCCESS) {
            ERROR_LOG("sample jpege process failed");
            return FAILED;
        }
        return SUCCESS;
    }

    if (dvppType == VPC_8K_RESIZE) {
        ret = sampleProcess.Resize8kProcess(dvppType);
        if (ret != SUCCESS) {
            ERROR_LOG("sample resize 8k process failed");
            return FAILED;
        }
        return SUCCESS;
    }

    ret = sampleProcess.JpegdProcess(dvppType);
    if (ret != SUCCESS) {
        ERROR_LOG("sample model process failed");
        return FAILED;
    }

    INFO_LOG("execute sample success");
    return SUCCESS;
}
