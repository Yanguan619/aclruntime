from mmengine.config import read_base
from ais_bench.benchmark.summarizers import DefaultPerfSummarizer
from ais_bench.benchmark.calculators import StagePerfMetricCalculator

summarizer = dict(
    type=DefaultPerfSummarizer,
    calculator=dict(
        type=StagePerfMetricCalculator,
        stats_list=["Average", "Min", "Max", "Median", "P75", "P90", "P99"],
        stages_list=["total", "stable"],
    )
)