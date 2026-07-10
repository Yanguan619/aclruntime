from pathlib import Path

import aclruntime


class Qwen35OM:
    def __init__(
        self,
        decoder_prefill_path: str,
        log_level=2,  # 1=DEBUG, 2=INFO, 3=WARNING, 4=ERROR
    ):
        self.options = aclruntime.session_options()
        self.options.weight_dir = str(Path(decoder_prefill_path).parent / "weight")
        self.options.without_graph = True
        self.options.log_level = log_level  # 设置日志级别
        self.model = aclruntime.InferenceSession(decoder_prefill_path, 0, self.options)


if __name__ == "__main__":
    import sys

    # 使用 --debug 参数启用内存监控
    log_level = 1 if "--debug" in sys.argv else 2
    Qwen35OM(
        decoder_prefill_path="/data/workspace/weight/Qwen3.5-2B-Edge/om-310p/decoder_model_prefill/decoder_model_prefill.om",
        log_level=log_level,
    )
