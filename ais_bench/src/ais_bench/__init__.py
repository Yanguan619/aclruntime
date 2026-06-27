# Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from typing import Annotated, Literal, Optional

import typer

from ais_bench.infer.args_adapter import AISBenchInferArgsAdapter
from ais_bench.infer.args_check import (
    check_acl_json_path_legality,
    check_aipp_config_path_legality,
    check_batchsize_valid,
    check_device_range_valid,
    check_dym_range_string,
    check_dym_string,
    check_input_path_legality,
    check_loop_size,
    check_nonnegative_integer,
    check_number_list,
    check_om_path_legality,
    check_output_path_legality,
    check_positive_integer,
)

app = typer.Typer()


@app.command()
def cli(
    model: str = typer.Option(
        ...,
        "--model",
        "-m",
        callback=check_om_path_legality,
        help="The path of the om model",
    ),
    input_path: Optional[str] = typer.Option(
        None,
        "--input",
        "-i",
        callback=check_input_path_legality,
        help="Input file or dir",
    ),
    output: Optional[str] = typer.Option(
        None,
        "--output",
        "-o",
        callback=check_output_path_legality,
        help="Inference data output path. The inference results are output to \
             the subdirectory named current date under given output path",
    ),
    output_dirname: Optional[str] = typer.Option(
        None,
        callback=check_output_path_legality,
        help="Actual output directory name. \
             Used with parameter output, cannot be used alone. \
             The inference result is output to subdirectory named by output_dirname \
             under  output path. such as --output_dirname 'tmp', \
             the final inference results are output to the folder of  {$output}/tmp",
    ),
    outfmt: Annotated[
        Literal["NPY", "BIN", "TXT"],
        typer.Option(help="Output file format (NPY or BIN or TXT)"),
    ] = "BIN",
    loop: str = typer.Option(
        "1",
        "--loop",
        "-l",
        callback=check_loop_size,
        help="The round of the PureInfer.",
    ),
    debug: bool = typer.Option(False, help="Debug switch,print model information"),
    device: str = typer.Option(
        "0",
        "--device",
        "-d",
        callback=check_device_range_valid,
        help="The NPU device ID to use.valid value range is [0, 255]",
    ),
    dym_batch: int = typer.Option(
        0, "--dymBatch", help="Dynamic batch size param，such as --dymBatch 2"
    ),
    dym_hw: Optional[str] = typer.Option(
        None,
        "--dymHW",
        callback=check_dym_string,
        help='Dynamic image size param, such as --dymHW "300,500"',
    ),
    dym_dims: Optional[str] = typer.Option(
        None,
        "--dymDims",
        callback=check_dym_string,
        help='Dynamic dims param, such as --dymDims "data:1,600;img_info:1,600"',
    ),
    dym_shape: Optional[str] = typer.Option(
        None,
        "--dymShape",
        callback=check_dym_string,
        help='Dynamic shape param, such as --dymShape "data:1,600;img_info:1,600"',
    ),
    output_size: Optional[str] = typer.Option(
        None,
        "--outputSize",
        callback=check_number_list,
        help="Output size for dynamic shape mode",
    ),
    auto_set_dymshape_mode: bool = typer.Option(False, help="Auto_set_dymshape_mode"),
    auto_set_dymdims_mode: bool = typer.Option(False, help="Auto_set_dymdims_mode"),
    batchsize: Optional[str] = typer.Option(
        None, callback=check_batchsize_valid, help="Batch size of input tensor"
    ),
    pure_data_type: Annotated[
        Literal["zero", "random"],
        typer.Option(
            help="Null data type for pure inference(zero or random)",
        ),
    ] = "zero",
    profiler: bool = typer.Option(False, help="Profiler switch"),
    dump: bool = typer.Option(False, help="Dump switch"),
    acl_json_path: Optional[str] = typer.Option(
        None,
        callback=check_acl_json_path_legality,
        help="Acl json path for profiling or dump",
    ),
    output_batchsize_axis: str = typer.Option(
        "0",
        callback=check_nonnegative_integer,
        help="Splitting axis number when outputing tensor results, \
             such as --output_batchsize_axis 1",
    ),
    run_mode: Annotated[
        Literal["array", "files", "tensor", "full"],
        typer.Option(
            help="Run mode",
        ),
    ] = "array",
    display_all_summary: bool = typer.Option(
        False, help="Display all summary include h2d d2h info"
    ),
    warmup_count: str = typer.Option(
        "1", callback=check_nonnegative_integer, help="Warmup count before inference"
    ),
    dym_shape_range: Optional[str] = typer.Option(
        None,
        "--dymShape_range",
        callback=check_dym_range_string,
        help='Dynamic shape range, such as --dymShape_range \
             "data:1,600~700;img_info:1,600-700"',
    ),
    aipp_config: Optional[str] = typer.Option(
        None,
        callback=check_aipp_config_path_legality,
        help="File type: .config, to set actual aipp params before infer",
    ),
    energy_consumption: bool = typer.Option(
        False, help="Obtain power consumption data for model inference"
    ),
    npu_id: str = typer.Option(
        "0",
        callback=check_device_range_valid,
        help="The NPU ID to use.valid value range is [0, 255]",
    ),
    backend: Annotated[Literal["trtexec"], typer.Option(help="Backend trtexec")] = "trtexec",
    perf: bool = typer.Option(False, help="Perf switch"),
    pipeline: bool = typer.Option(False, help="Pipeline switch"),
    profiler_rename: bool = typer.Option(True, help="Profiler rename switch"),
    dump_npy: bool = typer.Option(False, help="dump data convert to npy"),
    divide_input: bool = typer.Option(
        False,
        help="Input datas need to be divided to match multi devices or not, \
            --device should be list, default False",
    ),
    threads: str = typer.Option(
        "1",
        callback=check_positive_integer,
        help="Number of threads for computing. \
            need to set --pipeline when setting threads number to be more than one.",
    ),
):
    from ais_bench.infer.infer_process import infer_process

    args = AISBenchInferArgsAdapter(
        model=model,
        input=input_path,
        output=output,
        output_dirname=output_dirname,
        outfmt=outfmt,
        loop=loop,
        debug=debug,
        device=device,
        dym_batch=dym_batch,
        dym_hw=dym_hw,
        dym_dims=dym_dims,
        dym_shape=dym_shape,
        output_size=output_size,
        auto_set_dymshape_mode=auto_set_dymshape_mode,
        auto_set_dymdims_mode=auto_set_dymdims_mode,
        batchsize=batchsize,
        pure_data_type=pure_data_type,
        profiler=profiler,
        dump=dump,
        acl_json_path=acl_json_path,
        output_batchsize_axis=output_batchsize_axis,
        run_mode=run_mode,
        display_all_summary=display_all_summary,
        warmup_count=warmup_count,
        dym_shape_range=dym_shape_range,
        aipp_config=aipp_config,
        energy_consumption=energy_consumption,
        npu_id=npu_id,
        backend=backend,
        perf=perf,
        pipeline=pipeline,
        profiler_rename=profiler_rename,
        dump_npy=dump_npy,
        divide_input=divide_input,
        threads=threads,
    )
    ret = infer_process(args)
    exit(ret)


def main():
    app()
