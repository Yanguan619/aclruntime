# Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import json
import os
from dataclasses import dataclass

import numpy as np

from ais_bench.infer.common.path_security_check import ms_open
from ais_bench.infer.common.utils import logger


@dataclass
class ListInfo:
    min: float = 0.0
    max: float = 0.0
    mean: float = 0.0
    median: float = 0.0
    percentile: float = 0.0


@dataclass
class Result:
    npu_compute_time: ListInfo
    h2d_latency: ListInfo
    d2h_latency: ListInfo
    throughput: float
    scale: float
    batchsize: float


class Summary:
    def __init__(self):
        self.reset()
        self.infodict = {"filesinfo": {}}

    @staticmethod
    def merge_intervals(intervals):
        intervals.sort(key=lambda x: x[0])
        merged = []
        for interval in intervals:
            if not merged or merged[-1][1] < interval[0]:
                merged.append(list(interval))
            else:
                merged[-1][1] = max(merged[-1][1], interval[1])
        return merged

    @staticmethod
    def get_list_info(work_list, percentile_scale, merge=False, duration_list=None):
        list_info = ListInfo()
        if (
            merge
        ):  # work_list is a 2-dim vector each element is a pair containing start and end time
            n = len(work_list)
            if n == 0:
                raise RuntimeError("summary.get_list_info failed: inner error")
            merged_intervals = Summary.merge_intervals(work_list)
            sum_time = sum(end_time - start_time for start_time, end_time in merged_intervals)
            list_info = ListInfo(mean=sum_time / n)
            if duration_list is not None:
                list_info.min = np.min(duration_list)
                list_info.max = np.max(duration_list)
                list_info.median = np.median(duration_list)
                list_info.percentile = np.percentile(duration_list, percentile_scale)
        elif len(work_list) != 0:
            list_info = ListInfo(
                min=np.min(work_list),
                max=np.max(work_list),
                mean=np.mean(work_list),
                median=np.median(work_list),
                percentile=np.percentile(work_list, percentile_scale),
            )
        return list_info

    def reset(self):
        self.h2d_latency_list = []
        self.d2h_latency_list = []
        self.npu_compute_time_list = []
        self.npu_compute_time_interval_list = []
        self._batchsizes = []

    def add_batchsize(self, n: int):
        self._batchsizes.append(n)

    def add_sample_id_infiles(self, sample_id, infiles):
        if self.infodict["filesinfo"].get(sample_id) is None:
            self.infodict["filesinfo"][sample_id] = {"infiles": [], "outfiles": []}
        if len(self.infodict["filesinfo"][sample_id]["infiles"]) == 0:
            for files in infiles:
                self.infodict["filesinfo"][sample_id]["infiles"].append(files)

    def append_sample_id_outfile(self, sample_id, outfile):
        if self.infodict["filesinfo"].get(sample_id) is None:
            self.infodict["filesinfo"][sample_id] = {"infiles": [], "outfiles": []}
        self.infodict["filesinfo"][sample_id]["outfiles"].append(outfile)

    def add_args(self, args):
        self.infodict["args"] = args

    def record(self, result, multi_threads=False):
        if multi_threads:
            self.infodict["NPU_compute_time"] = {
                "min": result.npu_compute_time.min,
                "max": result.npu_compute_time.max,
                "mean": result.npu_compute_time.mean,
                "median": result.npu_compute_time.median,
                "P{}".format(result.scale): result.npu_compute_time.percentile,
                "count": len(self.npu_compute_time_interval_list),
            }
            self.infodict["H2D_latency"] = {
                "min": result.h2d_latency.min,
                "max": result.h2d_latency.max,
                "mean": result.h2d_latency.mean,
                "median": result.h2d_latency.median,
                "P{}".format(result.scale): result.h2d_latency.percentile,
                "count": len(self.h2d_latency_list),
            }
            self.infodict["D2H_latency"] = {
                "min": result.d2h_latency.min,
                "max": result.d2h_latency.max,
                "mean": result.d2h_latency.mean,
                "median": result.d2h_latency.median,
                "P{}".format(result.scale): result.d2h_latency.percentile,
                "count": len(self.d2h_latency_list),
            }
            self.infodict["npu_compute_time_list"] = self.npu_compute_time_interval_list
        else:
            self.infodict["NPU_compute_time"] = {
                "min": result.npu_compute_time.min,
                "max": result.npu_compute_time.max,
                "mean": result.npu_compute_time.mean,
                "median": result.npu_compute_time.median,
                "P{}".format(result.scale): result.npu_compute_time.percentile,
                "count": len(self.npu_compute_time_list),
            }
            self.infodict["H2D_latency"] = {
                "min": result.h2d_latency.min,
                "max": result.h2d_latency.max,
                "mean": result.h2d_latency.mean,
                "median": result.h2d_latency.median,
                "P{}".format(result.scale): result.h2d_latency.percentile,
                "count": len(self.h2d_latency_list),
            }
            self.infodict["D2H_latency"] = {
                "min": result.d2h_latency.min,
                "max": result.d2h_latency.max,
                "mean": result.d2h_latency.mean,
                "median": result.d2h_latency.median,
                "P{}".format(result.scale): result.d2h_latency.percentile,
                "count": len(self.d2h_latency_list),
            }
            self.infodict["npu_compute_time_list"] = self.npu_compute_time_list
        self.infodict["throughput"] = result.throughput
        self.infodict["pid"] = os.getpid()

    def display(self, result, display_all_summary, multi_threads):
        from tabulate import tabulate

        logger.info("-----------------Performance Summary------------------")
        if multi_threads:
            rows = [
                [
                    "NPU_compute_time (ms)",
                    f"{result.npu_compute_time.min}",
                    f"{result.npu_compute_time.max}",
                    f"{result.npu_compute_time.mean}",
                    f"{result.npu_compute_time.median}",
                    f"{result.npu_compute_time.percentile}",
                ]
            ]
            if display_all_summary:
                rows.insert(
                    0,
                    [
                        "H2D_latency (ms)",
                        f"{result.h2d_latency.min}",
                        f"{result.h2d_latency.max}",
                        f"{result.h2d_latency.mean}",
                        f"{result.h2d_latency.median}",
                        f"{result.h2d_latency.percentile}",
                    ],
                )
                rows.append(
                    [
                        "D2H_latency (ms)",
                        f"{result.d2h_latency.min}",
                        f"{result.d2h_latency.max}",
                        f"{result.d2h_latency.mean}",
                        f"{result.d2h_latency.median}",
                        f"{result.d2h_latency.percentile}",
                    ]
                )
            table = tabulate(
                rows,
                headers=[
                    "Metric",
                    "min",
                    "max",
                    "mean",
                    "median",
                    f"P{result.scale}",
                ],
                tablefmt="fancy_grid",
                floatfmt=".4f",
            )
            print(table)
        else:
            rows = [
                [
                    "NPU_compute_time (ms)",
                    result.npu_compute_time.min,
                    result.npu_compute_time.max,
                    result.npu_compute_time.mean,
                    result.npu_compute_time.median,
                    result.npu_compute_time.percentile,
                ]
            ]
            if display_all_summary:
                rows.insert(
                    0,
                    [
                        "H2D_latency (ms)",
                        result.h2d_latency.min,
                        result.h2d_latency.max,
                        result.h2d_latency.mean,
                        result.h2d_latency.median,
                        result.h2d_latency.percentile,
                    ],
                )
                rows.append(
                    [
                        "D2H_latency (ms)",
                        result.d2h_latency.min,
                        result.d2h_latency.max,
                        result.d2h_latency.mean,
                        result.d2h_latency.median,
                        result.d2h_latency.percentile,
                    ]
                )
            print(
                tabulate(
                    rows,
                    headers=[
                        "Metric",
                        "min",
                        "max",
                        "mean",
                        "median",
                        f"P{result.scale}",
                    ],
                    tablefmt="fancy_grid",
                    floatfmt=".4f",
                )
            )
        logger.info(
            "Throughput 1000*batchsize.mean({})/NPU_compute_time.mean({}): {}".format(
                result.batchsize, result.npu_compute_time.mean, result.throughput
            )
        )
        logger.info("------------------------------------------------------")

    def report(
        self,
        batchsize: float,
        output_prefix,
        display_all_summary=False,
        multi_threads=False,
    ):
        scale = 99

        if self.npu_compute_time_list and self.npu_compute_time_interval_list:
            logger.error(
                "npu_compute_time_list and npu_compute_time_interval_list exits at the same time"
            )
            raise Exception
        if self.npu_compute_time_list:
            npu_compute_time = Summary.get_list_info(self.npu_compute_time_list, scale)
        else:
            durations = [end - start for start, end in self.npu_compute_time_interval_list]
            npu_compute_time = Summary.get_list_info(
                self.npu_compute_time_interval_list, scale, True, durations
            )
        h2d_latency = Summary.get_list_info(self.h2d_latency_list, scale)
        d2h_latency = Summary.get_list_info(self.d2h_latency_list, scale)
        if self._batchsizes:
            batchsize = sum(self._batchsizes) / len(self._batchsizes)

        throughput = 1000 * batchsize / npu_compute_time.mean if npu_compute_time.mean != 0 else 0

        result = Result(
            npu_compute_time=npu_compute_time,
            d2h_latency=d2h_latency,
            h2d_latency=h2d_latency,
            throughput=throughput,
            scale=scale,
            batchsize=batchsize,
        )

        self.record(result, multi_threads)
        self.display(result, display_all_summary, multi_threads)

        if output_prefix is not None:
            with ms_open(output_prefix + "_summary.json", mode="w") as f:
                json.dump(self.infodict, f)


summary = Summary()
