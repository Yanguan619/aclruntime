import dataclasses
from dataclasses import dataclass


@dataclass
class AISBenchInferArgsAdapter:
    model: str
    input: str | None
    output: str | None
    output_dirname: str | None
    outfmt: str
    loop: str
    debug: bool
    device: str
    dym_batch: int
    dym_hw: str | None
    dym_dims: str | None
    dym_shape: str | None
    output_size: str | None
    auto_set_dymshape_mode: bool
    auto_set_dymdims_mode: bool
    batchsize: str | None
    pure_data_type: str
    profiler: bool
    dump: bool
    acl_json_path: str | None
    output_batchsize_axis: str
    run_mode: str
    display_all_summary: bool
    warmup_count: str
    dym_shape_range: str | None
    aipp_config: str | None
    energy_consumption: bool
    npu_id: str
    backend: str
    perf: bool
    pipeline: bool
    profiler_rename: bool
    dump_npy: bool
    divide_input: bool
    threads: str

    _FLAG_OVERRIDES = {
        "dym_batch": "--dymBatch",
        "dym_hw": "--dymHW",
        "dym_dims": "--dymDims",
        "dym_shape": "--dymShape",
        "output_size": "--outputSize",
        "dym_shape_range": "--dymShape_range",
    }

    def get_all_args_dict(self) -> dict:
        """逆序列化回 CLI flag → value 字典，用于子进程重新拼命令行。

        Callers:
            infer_process.py:regenerate_cmd         (msprof profiling)
            miscellaneous.py:regenerate_dymshape_cmd (dymshape range)
        """
        result = {}
        for field in dataclasses.fields(self):
            flag = self._FLAG_OVERRIDES.get(field.name, "--" + field.name)
            result[flag] = getattr(self, field.name)
        return result
