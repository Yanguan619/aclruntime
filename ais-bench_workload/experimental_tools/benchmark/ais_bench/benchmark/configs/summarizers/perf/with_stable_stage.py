from mmengine.config import read_base
from ais_bench.benchmark.summarizers import DefaultPerfSummarizer
from ais_bench.benchmark.calculators import StablePerfMetricCalculator

summarizer = dict(
    type=DefaultPerfSummarizer,
    calculator=dict(type=StablePerfMetricCalculator)
)