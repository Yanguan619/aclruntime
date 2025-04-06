from ais_bench.benchmark.models import MindieLLMAPI

models = [
    dict(
        type=MindieLLMAPI,
        abbr='mindie-llm-api',
        max_out_len = 1024,  # 推理接口调用时设定的最大输出长度，建议与下面的output_length相同

        world_size = 2,  # 本次推理使用的卡总数

        block_size = 128,  # block size是warm up时进行预先计算内存所需的参数
        model_name = "qwen",  # 模型名称
        data_type = "bf16",  # 模型配置数据类型
        weight_dir = "/data1/qwen2-7b-instruct-cp",  # 模型权重路径
        max_position_embedding = 5120,  # 模型预先分配内存所需的参数
        is_chat_model = False,  # 是否使用chat模板
        decode_batch_size = 32,  # decode阶段的batchsize，与数据集推理时设定的batchsize相同
        prefill_batch_size = 0,  # prefill阶段的batchsize

        dp = -1,  # dp tp sp moe_tp pp microbatch_size moe_ep 模型并行参数
        tp = -1,
        sp = -1,
        moe_tp = -1,
        pp = -1,
        microbatch_size = -1,
        moe_ep = -1,
        trust_remote_code = False,  # 是否信任远端代码
        ignore_eos = False,  # 是否忽略推理终止符
        input_length = 4096,  # warm_up参数，input长度
        output_length = 1024,  # warm_up参数，output长度
        run_cfg = dict(num_gpus=2, num_procs=2)  # 多卡/多机多卡 参数，NPU侧使用torchrun拉起任务
    )
]
