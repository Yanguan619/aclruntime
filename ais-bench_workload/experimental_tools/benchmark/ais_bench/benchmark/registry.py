import importlib
from typing import Callable, List, Optional, Type, Union

from mmengine.registry import METRICS as MMENGINE_METRICS
from mmengine.registry import Registry as OriginalRegistry
from ais_bench.benchmark.global_consts import CUSTOM_PACKAGE_DIR

def load_class(class_path):
    """动态加载类路径并返回类对象"""
    try:
        parts = class_path.split('.')
        module_name = '.'.join(parts[:-1])
        class_name = parts[-1]

        module = importlib.import_module(module_name)
        return getattr(module, class_name)
    except (ImportError, AttributeError) as e:
        raise ValueError(f"无法加载类 {class_path}: {e}") from e


class Registry(OriginalRegistry):

    # override the default force behavior
    def register_module(
            self,
            name: Optional[Union[str, List[str]]] = None,
            force: bool = True,
            module: Optional[Type] = None) -> Union[type, Callable]:
        return super().register_module(name, force, module)


PARTITIONERS = Registry('partitioner', locations=['ais_bench.benchmark.partitioners', f'{CUSTOM_PACKAGE_DIR}.partitioners'])
RUNNERS = Registry('runner', locations=['ais_bench.benchmark.runners', f'{CUSTOM_PACKAGE_DIR}.runners'])
TASKS = Registry('task', locations=['ais_bench.benchmark.tasks', f'{CUSTOM_PACKAGE_DIR}.tasks'])
MODELS = Registry('model', locations=['ais_bench.benchmark.models', f'{CUSTOM_PACKAGE_DIR}.models'])
# TODO: LOAD_DATASET -> DATASETS
LOAD_DATASET = Registry('load_dataset', locations=['ais_bench.benchmark.datasets', f'{CUSTOM_PACKAGE_DIR}.datasets'])
TEXT_POSTPROCESSORS = Registry(
    'text_postprocessors', locations=['ais_bench.benchmark.utils.text_postprocessors', f'{CUSTOM_PACKAGE_DIR}.utils.text_postprocessors'])
DICT_POSTPROCESSORS = Registry(
    'dict_postprocessors', locations=['ais_bench.benchmark.utils.dict_postprocessors', f'{CUSTOM_PACKAGE_DIR}.utils.dict_postprocessors'])

EVALUATORS = Registry('evaluators', locations=['ais_bench.benchmark.evaluators', f'{CUSTOM_PACKAGE_DIR}.evaluators'])

ICL_INFERENCERS = Registry('icl_inferencers',
                           locations=['ais_bench.benchmark.openicl.icl_inferencer', f'{CUSTOM_PACKAGE_DIR}.openicl.icl_inferencer'])
ICL_RETRIEVERS = Registry('icl_retrievers',
                          locations=['ais_bench.benchmark.openicl.icl_retriever', f'{CUSTOM_PACKAGE_DIR}.openicl.icl_retriever'])
ICL_DATASET_READERS = Registry(
    'icl_dataset_readers',
    locations=['ais_bench.benchmark.openicl.icl_dataset_reader', f'{CUSTOM_PACKAGE_DIR}.openicl.icl_dataset_reader'])
ICL_PROMPT_TEMPLATES = Registry(
    'icl_prompt_templates',
    locations=['ais_bench.benchmark.openicl.icl_prompt_template', f'{CUSTOM_PACKAGE_DIR}.openicl.icl_prompt_template'])
ICL_EVALUATORS = Registry('icl_evaluators',
                          locations=['ais_bench.benchmark.openicl.icl_evaluator', f'{CUSTOM_PACKAGE_DIR}.openicl.icl_evaluator'])
METRICS = Registry('metric',
                   parent=MMENGINE_METRICS,
                   locations=['ais_bench.benchmark.metrics', f'{CUSTOM_PACKAGE_DIR}.metrics'])
TOT_WRAPPER = Registry('tot_wrapper', locations=['ais_bench.benchmark.datasets', f'{CUSTOM_PACKAGE_DIR}.datasets'])


def build_from_cfg(cfg):
    """A helper function that builds object with MMEngine's new config."""
    return PARTITIONERS.build(cfg)
