# encoding: utf-8
# 安装order中分组配置的顺序执行测试用例,True并发执行测试,False串行执行测试,分组内的测试用例不分先后顺序
import oec
test_sequence = {
    ("运行环境", "环境信息"): False,
    ("运行环境", "运行依赖"): False,
    ("运行环境", "CANN信息"): False,
    ("应用开发", "基础功能"): False,
    ("应用开发", "算子加速库"): True,
    ("应用开发", "媒体处理"): False,
    ("算子", "算子编译"): True,
    ("算子", "算子开发"): True,
    ("模型开发", "模型编译"): False,
    ("模型开发", "模型调优"): False,
    ("模型开发", "集合通信"): False,
    ("集成测试", "ATB"): False,
    ("集成测试", "离线推理"): False,
    ("集成测试", "在线训练"): False,
    ("运行环境", "CANN安装卸载"): False,
}


offering = {
    "default":["all"],
    "cann-env":[oec.env],
    "cann-package":[oec.env_os, oec.env_drv, oec.combo_package],
    "cann-media":[oec.env, oec.media],
    "cann-model-dev":[oec.env, oec.model_dev],
    "cann-hccl":[oec.env, oec.hccl],
    "cann-atb":[oec.env, oec.atb],
}