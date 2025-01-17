/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include "Base/ModelInfer/DynamicAippConfig.h"

namespace Base {
DynamicAippConfig::DynamicAippConfig()
{
    isActivated = false;
    modelOK = false;
    srcImageSizeH = 0;
    srcImageSizeW = 0;
    axSwapSwitch = 0;
    rbuvSwapSwitch = 0;
    maxBatchSize = 0;
    cscParams.cscSwitch = 0;
    cscParams.cscMatrixR0C0 = 0;
    cscParams.cscMatrixR0C1 = 0;
    cscParams.cscMatrixR0C2 = 0;
    cscParams.cscMatrixR1C0 = 0;
    cscParams.cscMatrixR1C1 = 0;
    cscParams.cscMatrixR1C2 = 0;
    cscParams.cscMatrixR2C0 = 0;
    cscParams.cscMatrixR2C1 = 0;
    cscParams.cscMatrixR2C2 = 0;
    cscParams.cscOutputBias0 = 0;
    cscParams.cscOutputBias1 = 0;
    cscParams.cscOutputBias2 = 0;
    cscParams.cscInputBias0 = 0;
    cscParams.cscInputBias1 = 0;
    cscParams.cscInputBias2 = 0;
};

DynamicAippConfig::~DynamicAippConfig()
{
    cropParams.clear();
    paddingParams.clear();
    dtcPixelMeanParams.clear();
    dtcPixelMinParams.clear();
    pixelVarReciParams.clear();
}

bool DynamicAippConfig::IsActivated()
{
    return isActivated;
}

bool DynamicAippConfig::ModelIsLegal()
{
    return modelOK;
}

void DynamicAippConfig::ActivateConfig()
{
    isActivated = true;
}

void DynamicAippConfig::ActivateModel()
{
    modelOK = true;
}

APP_ERROR DynamicAippConfig::SetMaxBatchSize(uint64_t maxBsParams)
{
    maxBatchSize = maxBsParams;
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetInputFormat(std::string iptFmt)
{
    inputFormat = iptFmt;
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetSrcImageSize(std::vector<int> srcImageSize)
{
    srcImageSizeW = srcImageSize[0];
    srcImageSizeH = srcImageSize[1];
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetRbuvSwapSwitch(int rsSwitch)
{
    rbuvSwapSwitch = rsSwitch;
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetAxSwapSwitch(int asSwitch)
{
    axSwapSwitch = asSwitch;
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetCscParams(std::vector<int> cscInputParams)
{
    cscParams.cscSwitch = cscInputParams[CSC_SWITCH];
    cscParams.cscMatrixR0C0 = cscInputParams[CSC_MATRIX_R0C0];
    cscParams.cscMatrixR0C1 = cscInputParams[CSC_MATRIX_R0C1];
    cscParams.cscMatrixR0C2 = cscInputParams[CSC_MATRIX_R0C2];
    cscParams.cscMatrixR1C0 = cscInputParams[CSC_MATRIX_R1C0];
    cscParams.cscMatrixR1C1 = cscInputParams[CSC_MATRIX_R1C1];
    cscParams.cscMatrixR1C2 = cscInputParams[CSC_MATRIX_R1C2];
    cscParams.cscMatrixR2C0 = cscInputParams[CSC_MATRIX_R2C0];
    cscParams.cscMatrixR2C1 = cscInputParams[CSC_MATRIX_R2C1];
    cscParams.cscMatrixR2C2 = cscInputParams[CSC_MATRIX_R2C2];
    cscParams.cscOutputBias0 = cscInputParams[CSC_OUTPUT_BIAS0];
    cscParams.cscOutputBias1 = cscInputParams[CSC_OUTPUT_BIAS1];
    cscParams.cscOutputBias2 = cscInputParams[CSC_OUTPUT_BIAS2];
    cscParams.cscInputBias0 = cscInputParams[CSC_INPUT_BIAS0];
    cscParams.cscInputBias1 = cscInputParams[CSC_INPUT_BIAS1];
    cscParams.cscInputBias2 = cscInputParams[CSC_INPUT_BIAS2];
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetCropParams(std::vector<int> cropInputParams)
{
    CropParams tmpCrop;
    tmpCrop.cropSwitch = cropInputParams[INPUT_PARAM0_INDEX];
    tmpCrop.loadStartPosW = cropInputParams[INPUT_PARAM1_INDEX];
    tmpCrop.loadStartPosH = cropInputParams[INPUT_PARAM2_INDEX];
    tmpCrop.cropSizeW = cropInputParams[INPUT_PARAM3_INDEX];
    tmpCrop.cropSizeH = cropInputParams[INPUT_PARAM4_INDEX];
    for (size_t batchIndex = 0; batchIndex < maxBatchSize; batchIndex++) {
        cropParams.insert(std::make_pair(batchIndex, tmpCrop));
    }
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetPaddingParams(std::vector<int> padInputParams)
{
    PaddingParams tmpPad;
    tmpPad.paddingSwitch = padInputParams[INPUT_PARAM0_INDEX];
    tmpPad.paddingSizeTop = padInputParams[INPUT_PARAM1_INDEX];
    tmpPad.paddingSizeBottom = padInputParams[INPUT_PARAM2_INDEX];
    tmpPad.paddingSizeLeft = padInputParams[INPUT_PARAM3_INDEX];
    tmpPad.paddingSizeRight = padInputParams[INPUT_PARAM4_INDEX];
    for (size_t batchIndex = 0; batchIndex < maxBatchSize; batchIndex++) {
        paddingParams.insert(std::make_pair(batchIndex, tmpPad));
    }
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetDtcPixelMean(std::vector<int> meanInputParams)
{
    DtcPixelMean tmpMean;
    tmpMean.dtcPixelMeanChn0 = meanInputParams[PIXEL_CHN0_INDEX];
    tmpMean.dtcPixelMeanChn1 = meanInputParams[PIXEL_CHN1_INDEX];
    tmpMean.dtcPixelMeanChn2 = meanInputParams[PIXEL_CHN2_INDEX];
    tmpMean.dtcPixelMeanChn3 = meanInputParams[PIXEL_CHN3_INDEX];
    for (size_t batchIndex = 0; batchIndex < maxBatchSize; batchIndex++) {
        dtcPixelMeanParams.insert(std::make_pair(batchIndex, tmpMean));
    }
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetDtcPixelMin(std::vector<float> minInputParams)
{
    DtcPixelMin tmpMin;
    tmpMin.dtcPixelMinChn0 = minInputParams[PIXEL_CHN0_INDEX];
    tmpMin.dtcPixelMinChn1 = minInputParams[PIXEL_CHN1_INDEX];
    tmpMin.dtcPixelMinChn2 = minInputParams[PIXEL_CHN2_INDEX];
    tmpMin.dtcPixelMinChn3 = minInputParams[PIXEL_CHN3_INDEX];
    for (size_t batchIndex = 0; batchIndex < maxBatchSize; batchIndex++) {
        dtcPixelMinParams.insert(std::make_pair(batchIndex, tmpMin));
    }
    return APP_ERR_OK;
}

APP_ERROR DynamicAippConfig::SetPixelVarReci(std::vector<float> reciInputParams)
{
    PixelVarReci tmpReci;
    tmpReci.dtcPixelVarReciChn0 = reciInputParams[PIXEL_CHN0_INDEX];
    tmpReci.dtcPixelVarReciChn1 = reciInputParams[PIXEL_CHN1_INDEX];
    tmpReci.dtcPixelVarReciChn2 = reciInputParams[PIXEL_CHN2_INDEX];
    tmpReci.dtcPixelVarReciChn3 = reciInputParams[PIXEL_CHN3_INDEX];
    for (size_t batchIndex = 0; batchIndex < maxBatchSize; batchIndex++) {
        pixelVarReciParams.insert(std::make_pair(batchIndex, tmpReci));
    }
    return APP_ERR_OK;
}

uint64_t DynamicAippConfig::GetMaxBatchSize()
{
    return maxBatchSize;
}

std::string DynamicAippConfig::GetInputFormat()
{
    return inputFormat;
}

int32_t DynamicAippConfig::GetSrcImageSizeW()
{
    return srcImageSizeW;
}

int32_t DynamicAippConfig::GetSrcImageSizeH()
{
    return srcImageSizeH;
}

int8_t DynamicAippConfig::GetRbuvSwapSwitch()
{
    return rbuvSwapSwitch;
}

int8_t DynamicAippConfig::GetAxSwapSwitch()
{
    return axSwapSwitch;
}

CscParams DynamicAippConfig::GetCscParams()
{
    return cscParams;
}

std::unordered_map<uint64_t, CropParams> DynamicAippConfig::GetCropParams()
{
    return cropParams;
}

std::unordered_map<uint64_t, PaddingParams> DynamicAippConfig::GetPaddingParams()
{
    return paddingParams;
}

std::unordered_map<uint64_t, DtcPixelMean> DynamicAippConfig::GetDtcPixelMean()
{
    return dtcPixelMeanParams;
}

std::unordered_map<uint64_t, DtcPixelMin> DynamicAippConfig::GetDtcPixelMin()
{
    return dtcPixelMinParams;
}

std::unordered_map<uint64_t, PixelVarReci> DynamicAippConfig::GetPixelVarReci()
{
    return pixelVarReciParams;
}
}   // namespace Base