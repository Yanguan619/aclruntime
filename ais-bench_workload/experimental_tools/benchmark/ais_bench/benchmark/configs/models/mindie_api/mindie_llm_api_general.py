from ais_bench.benchmark.models import MindieLLMAPI

models = [
    dict(
        type=MindieLLMAPI,
        abbr='mindie-llm-api',
        max_out_len = 1024,

        rank = 0,
        local_rank = 0,
        world_size = 2,

        block_size = 128,
        model_name = "qwen",
        data_type = "bf16",
        weight_dir = "/data1/qwen2-7b-instruct-cp",
        max_position_embedding = 5120,
        is_chat_model = False,
        decode_batch_size = 32,
        prefill_batch_size = 0,
        kw_args = "",
        dp = -1,
        tp = -1,
        sp = -1,
        moe_tp = -1,
        pp = -1,
        microbatch_size = -1,
        moe_ep = -1,
        trust_remote_code = False,
        ignore_eos = False,
        input_length = 4096,
        output_length = 1024,
        run_cfg = dict(num_gpus=2, num_procs=2)
    )
]
