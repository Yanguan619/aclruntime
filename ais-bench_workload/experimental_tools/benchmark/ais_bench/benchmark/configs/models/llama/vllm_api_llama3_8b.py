from ais_bench.benchmark.models import VLLMCustomAPI

models = [
    dict(
        type=VLLMCustomAPI,
        abbr='vllm-api-llama3-8b', # for llama3 llama3.1 llama3.2
        path='', # VLLMCustomAPI auto get path from server
        max_seq_len = 4096,
        query_per_second = 1,
        rpm_verbose = False,
        retry = 2,
        host_ip = "localhost",
        host_port = 8080,
        enable_ssl = False,
        max_out_len=512,
    )
]
