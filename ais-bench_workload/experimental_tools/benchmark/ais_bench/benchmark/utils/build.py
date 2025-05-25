import re
import os
import copy
import ipaddress

from mmengine.config import ConfigDict

from ais_bench.benchmark.registry import LOAD_DATASET, MODELS, PERF_METRIC_CALCULATORS


def validate_model_cfg(model_cfg: dict) -> dict:
    errors = {}

    def check(condition, key, message):
        if not condition:
            errors[key] = message

    for key, value in model_cfg.items():
        if key == "attr":
            check(value in ("local", "service"), key, "attr must be 'local' or 'service'")

        elif key == "abbr":
            check(isinstance(value, str) and re.fullmatch(r'[A-Za-z\-]+', value), key,
                  "abbr must contain only letters and hyphens (e.g., 'vllm-api-general-chat')")

        elif key == "path":
            check(isinstance(value, str) and os.path.exists(value), key,
                  f"path is not accessible or does not exist: {value}")

        elif key == "model":
            check(isinstance(value, str) and value.strip() != "", key,
                  "model must be a non-empty string")

        elif key == "max_seq_len":
            check(isinstance(value, int) and 0 < value <= 131072, key,
                  "max_seq_len must be an integer in the range (0, 131072]")

        elif key == "request_rate":
            check(isinstance(value, (int, float)) and 0 <= value <= 100000, key,
                  "request_rate must be a number in the range [0, 100000]")

        elif key == "retry":
            check(isinstance(value, int) and 0 <= value <= 1000, key,
                  "retry must be an integer in the range [0, 1000]")

        elif key == "host_ip":
            if value == "localhost":
                continue
            try:
                ipaddress.ip_address(value)
            except ValueError:
                errors[key] = "host_ip must be a valid IPv4 or IPv6 address"

        elif key == "host_port":
            check(isinstance(value, int) and (0 < value < 65536), key,
                  "host_port must be a valid port number in the range (0, 65536)")

        elif key == "max_out_len":
            check(isinstance(value, int) and 0 < value <= 131072, key,
                  "max_out_len must be an integer in the range (0, 131072]")

        elif key == "batch_size":
            check(isinstance(value, int) and 0 < value <= 64000, key,
                  "batch_size must be an integer in the range (0, 64000]")

        elif key == "generation_kwargs":
            check(isinstance(value, dict), key,
                  "generation_kwargs must be a dictionary")

        elif key == "type":
            # Not configurable; skip validation
            continue

    return errors
    

def build_dataset_from_cfg(dataset_cfg: ConfigDict):
    dataset_cfg = copy.deepcopy(dataset_cfg)
    dataset_cfg.pop('infer_cfg', None)
    dataset_cfg.pop('eval_cfg', None)
    dataset_cfg.pop('abbr', None)
    return LOAD_DATASET.build(dataset_cfg)


def build_model_from_cfg(model_cfg: ConfigDict):
    model_cfg = copy.deepcopy(model_cfg)
    errors = validate_model_cfg(model_cfg)
    if errors:
        raise ValueError(f"Model build failed with the following errors: {errors}")
    model_cfg.pop('run_cfg', None)
    model_cfg.pop('max_out_len', None)
    model_cfg.pop('batch_size', None)
    model_cfg.pop('abbr', None)
    model_cfg.pop('attr', None)
    model_cfg.pop('summarizer_abbr', None)
    model_cfg.pop('pred_postprocessor', None)
    model_cfg.pop('min_out_len', None)
    return MODELS.build(model_cfg)


def build_perf_metric_calculator_from_cfg(metric_cfg: ConfigDict, perf_details: dict):
    metric_cfg = copy.deepcopy(metric_cfg)
    metric_cfg['perf_details'] = perf_details
    return PERF_METRIC_CALCULATORS.build(metric_cfg)