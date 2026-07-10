#include <gtest/gtest.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "Base/ModelInfer/DynamicAippConfig.h"

using namespace Base;

namespace {

TEST(DynamicAippConfigTest, BasicActivationAndModelLegal) {
    DynamicAippConfig config;
    EXPECT_FALSE(config.IsActivated());
    EXPECT_FALSE(config.ModelIsLegal());
    config.ActivateConfig();
    config.ActivateModel();
    EXPECT_TRUE(config.IsActivated());
    EXPECT_TRUE(config.ModelIsLegal());
}

TEST(DynamicAippConfigTest, SetAndGetMaxBatchSize) {
    DynamicAippConfig config;
    EXPECT_EQ(config.SetMaxBatchSize(8), APP_ERR_OK);
    EXPECT_EQ(config.GetMaxBatchSize(), 8u);
}

TEST(DynamicAippConfigTest, SetAndGetInputFormat) {
    DynamicAippConfig config;
    EXPECT_EQ(config.SetInputFormat("YUV"), APP_ERR_OK);
    EXPECT_EQ(config.GetInputFormat(), "YUV");
}

TEST(DynamicAippConfigTest, SetAndGetSrcImageSize) {
    DynamicAippConfig config;
    std::vector<int> size = {1920, 1080};
    EXPECT_EQ(config.SetSrcImageSize(size), APP_ERR_OK);
    EXPECT_EQ(config.GetSrcImageSizeW(), 1920);
    EXPECT_EQ(config.GetSrcImageSizeH(), 1080);
}

TEST(DynamicAippConfigTest, SetAndGetRbuvAxSwapSwitch) {
    DynamicAippConfig config;
    EXPECT_EQ(config.SetRbuvSwapSwitch(1), APP_ERR_OK);
    EXPECT_EQ(config.SetAxSwapSwitch(2), APP_ERR_OK);
    EXPECT_EQ(config.GetRbuvSwapSwitch(), 1);
    EXPECT_EQ(config.GetAxSwapSwitch(), 2);
}

TEST(DynamicAippConfigTest, SetAndGetCscParams) {
    DynamicAippConfig config;
    std::vector<int> csc(16, 1);
    EXPECT_EQ(config.SetCscParams(csc), APP_ERR_OK);
    auto params = config.GetCscParams();
    EXPECT_EQ(params.cscSwitch, 1);
    EXPECT_EQ(params.cscMatrixR0C0, 1);
    EXPECT_EQ(params.cscMatrixR2C2, 1);
    EXPECT_EQ(params.cscInputBias2, 1);
}

TEST(DynamicAippConfigTest, SetAndGetCropParams) {
    DynamicAippConfig config;
    config.SetMaxBatchSize(2);
    std::vector<int> crop = {1, 2, 3, 4, 5};
    EXPECT_EQ(config.SetCropParams(crop), APP_ERR_OK);
    auto map = config.GetCropParams();
    ASSERT_EQ(map.size(), 2u);
    for (const auto& kv : map) {
        EXPECT_EQ(kv.second.cropSwitch, 1);
        EXPECT_EQ(kv.second.loadStartPosW, 2);
        EXPECT_EQ(kv.second.loadStartPosH, 3);
        EXPECT_EQ(kv.second.cropSizeW, 4);
        EXPECT_EQ(kv.second.cropSizeH, 5);
    }
}

TEST(DynamicAippConfigTest, SetAndGetPaddingParams) {
    DynamicAippConfig config;
    config.SetMaxBatchSize(1);
    std::vector<int> pad = {1, 2, 3, 4, 5};
    EXPECT_EQ(config.SetPaddingParams(pad), APP_ERR_OK);
    auto map = config.GetPaddingParams();
    ASSERT_EQ(map.size(), 1u);
    auto val = map.begin()->second;
    EXPECT_EQ(val.paddingSwitch, 1);
    EXPECT_EQ(val.paddingSizeTop, 2);
    EXPECT_EQ(val.paddingSizeBottom, 3);
    EXPECT_EQ(val.paddingSizeLeft, 4);
    EXPECT_EQ(val.paddingSizeRight, 5);
}

TEST(DynamicAippConfigTest, SetAndGetDtcPixelMean) {
    DynamicAippConfig config;
    config.SetMaxBatchSize(1);
    std::vector<int> mean = {10, 20, 30, 40};
    EXPECT_EQ(config.SetDtcPixelMean(mean), APP_ERR_OK);
    auto map = config.GetDtcPixelMean();
    ASSERT_EQ(map.size(), 1u);
    auto val = map.begin()->second;
    EXPECT_EQ(val.dtcPixelMeanChn0, 10);
    EXPECT_EQ(val.dtcPixelMeanChn1, 20);
    EXPECT_EQ(val.dtcPixelMeanChn2, 30);
    EXPECT_EQ(val.dtcPixelMeanChn3, 40);
}

TEST(DynamicAippConfigTest, SetAndGetDtcPixelMin) {
    DynamicAippConfig config;
    config.SetMaxBatchSize(1);
    std::vector<float> min = {1.1f, 2.2f, 3.3f, 4.4f};
    EXPECT_EQ(config.SetDtcPixelMin(min), APP_ERR_OK);
    auto map = config.GetDtcPixelMin();
    ASSERT_EQ(map.size(), 1u);
    auto val = map.begin()->second;
    EXPECT_FLOAT_EQ(val.dtcPixelMinChn0, 1.1f);
    EXPECT_FLOAT_EQ(val.dtcPixelMinChn1, 2.2f);
    EXPECT_FLOAT_EQ(val.dtcPixelMinChn2, 3.3f);
    EXPECT_FLOAT_EQ(val.dtcPixelMinChn3, 4.4f);
}

TEST(DynamicAippConfigTest, SetAndGetPixelVarReci) {
    DynamicAippConfig config;
    config.SetMaxBatchSize(1);
    std::vector<float> reci = {0.1f, 0.2f, 0.3f, 0.4f};
    EXPECT_EQ(config.SetPixelVarReci(reci), APP_ERR_OK);
    auto map = config.GetPixelVarReci();
    ASSERT_EQ(map.size(), 1u);
    auto val = map.begin()->second;
    EXPECT_FLOAT_EQ(val.dtcPixelVarReciChn0, 0.1f);
    EXPECT_FLOAT_EQ(val.dtcPixelVarReciChn1, 0.2f);
    EXPECT_FLOAT_EQ(val.dtcPixelVarReciChn2, 0.3f);
    EXPECT_FLOAT_EQ(val.dtcPixelVarReciChn3, 0.4f);
}

TEST(DynamicAippConfigTest, DestructorCoversClear) {
    DynamicAippConfig* config = new DynamicAippConfig();
    config->SetMaxBatchSize(1);
    config->SetCropParams({1, 2, 3, 4, 5});
    config->SetPaddingParams({1, 2, 3, 4, 5});
    config->SetDtcPixelMean({1, 2, 3, 4});
    config->SetDtcPixelMin({1.0f, 2.0f, 3.0f, 4.0f});
    config->SetPixelVarReci({1.0f, 2.0f, 3.0f, 4.0f});
    delete config;  // 覆盖析构
}

}  // namespace
