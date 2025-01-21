# flake8: noqa
# yapf: disable
import os
from typing import List, Tuple, Union

import tabulate
from mmengine.config import Config

from ais_bench.benchmark.datasets.custom import make_custom_dataset_config
from ais_bench.benchmark.models import (VLLM, HuggingFace, HuggingFaceBaseModel,
                                HuggingFaceCausalLM, HuggingFaceChatGLM3,
                                HuggingFacewithChatTemplate,
                                TurboMindModelwithChatTemplate,
                                VLLMwithChatTemplate)
from ais_bench.benchmark.partitioners import NaivePartitioner, NumWorkerPartitioner
from ais_bench.benchmark.runners import DLCRunner, LocalRunner, SlurmRunner
from ais_bench.benchmark.tasks import OpenICLEvalTask, OpenICLInferTask
from ais_bench.benchmark.utils import get_logger, match_files

logger = get_logger()

def match_cfg_file(workdir: Union[str, List[str]],
                   pattern: Union[str, List[str]]) -> List[Tuple[str, str]]:
    """Match the config file in workdir recursively given the pattern.

    Additionally, if the pattern itself points to an existing file, it will be
    directly returned.
    """
    def _mf_with_multi_workdirs(workdir, pattern, fuzzy=False):
        if isinstance(workdir, str):
            workdir = [workdir]
        files = []
        for wd in workdir:
            files += match_files(wd, pattern, fuzzy=fuzzy)
        return files

    if isinstance(pattern, str):
        pattern = [pattern]
    pattern = [p + '.py' if not p.endswith('.py') else p for p in pattern]
    files = _mf_with_multi_workdirs(workdir, pattern, fuzzy=False)
    if len(files) != len(pattern):
        nomatched = []
        ambiguous = []
        ambiguous_return_list = []
        err_msg = ('The provided pattern matches 0 or more than one '
                   'config. Please verify your pattern and try again. '
                   'You may use tools/list_configs.py to list or '
                   'locate the configurations.\n')
        for p in pattern:
            files_ = _mf_with_multi_workdirs(workdir, p, fuzzy=False)
            if len(files_) == 0:
                nomatched.append([p[:-3]])
            elif len(files_) > 1:
                ambiguous.append([p[:-3], '\n'.join(f[1] for f in files_)])
                ambiguous_return_list.append(files_[0])
        if nomatched:
            table = [['Not matched patterns'], *nomatched]
            err_msg += tabulate.tabulate(table,
                                         headers='firstrow',
                                         tablefmt='psql')
        if ambiguous:
            table = [['Ambiguous patterns', 'Matched files'], *ambiguous]
            warning_msg = 'Found ambiguous patterns, using the first matched config.\n'
            warning_msg += tabulate.tabulate(table,
                                         headers='firstrow',
                                         tablefmt='psql')
            logger.warning(warning_msg)
            return ambiguous_return_list

        raise ValueError(err_msg)
    return files


def try_fill_in_custom_cfgs(config):
    for i, dataset in enumerate(config['datasets']):
        if 'type' not in dataset:
            config['datasets'][i] = make_custom_dataset_config(dataset)
    if 'model_dataset_combinations' not in config:
        return config
    for mdc in config['model_dataset_combinations']:
        for i, dataset in enumerate(mdc['datasets']):
            if 'type' not in dataset:
                mdc['datasets'][i] = make_custom_dataset_config(dataset)
    return config


def get_config_from_arg(args) -> Config:
    """Get the config object given args.

    Only a few argument combinations are accepted (priority from high to low)
    1. args.config
    2. args.models and args.datasets
    3. Huggingface parameter groups and args.datasets
    """

    if args.config:
        config = Config.fromfile(args.config, format_python_code=False)
        return config

    # parse dataset args
    if not args.datasets and not args.custom_dataset_path:
        raise ValueError('You must specify "--datasets" or "--custom-dataset-path" if you do not specify a config file path.')
    datasets = []
    if args.datasets:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        parent_dir = os.path.dirname(script_dir)
        default_configs_dir = os.path.join(parent_dir, 'configs')
        datasets_dir = [
            os.path.join(args.config_dir, 'datasets'),
            os.path.join(args.config_dir, 'dataset_collections'),
            os.path.join(default_configs_dir, './datasets'),
            os.path.join(default_configs_dir, './dataset_collections')

        ]
        for dataset_arg in args.datasets:
            if '/' in dataset_arg:
                dataset_name, dataset_suffix = dataset_arg.split('/', 1)
                dataset_key_suffix = dataset_suffix
            else:
                dataset_name = dataset_arg
                dataset_key_suffix = '_datasets'

            for dataset in match_cfg_file(datasets_dir, [dataset_name]):
                logger.info(f'Loading {dataset[0]}: {dataset[1]}')
                cfg = Config.fromfile(dataset[1])
                for k in cfg.keys():
                    if k.endswith(dataset_key_suffix):
                        datasets += cfg[k]
    else:
        raise ValueError('You must specify "--datasets"')
    # parse model args
    if not args.models:
        raise ValueError('You must specify a config file path, or specify --models and --datasets.')
    models = []
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(script_dir)
    default_configs_dir = os.path.join(parent_dir, 'configs')
    models_dir = [
        os.path.join(args.config_dir, 'models'),
        os.path.join(default_configs_dir, './models'),

    ]
    if args.models:
        for model_arg in args.models:
            for model in match_cfg_file(models_dir, [model_arg]):
                logger.info(f'Loading {model[0]}: {model[1]}')
                cfg = Config.fromfile(model[1])
                if 'models' not in cfg:
                    raise ValueError(f'Config file {model[1]} does not contain "models" field')
                models += cfg['models']
    else:
        raise ValueError('You must specify "--models"')

    # parse summarizer args
    summarizer_arg = args.summarizer if args.summarizer is not None else 'example'
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(script_dir)
    default_configs_dir = os.path.join(parent_dir, 'configs')
    summarizers_dir = [
        os.path.join(args.config_dir, 'summarizers'),
        os.path.join(default_configs_dir, './summarizers'),

    ]

    # Check if summarizer_arg contains '/'
    if '/' in summarizer_arg:
        # If it contains '/', split the string by '/'
        # and use the second part as the configuration key
        summarizer_file, summarizer_key = summarizer_arg.split('/', 1)
    else:
        # If it does not contain '/', keep the original logic unchanged
        summarizer_key = 'summarizer'
        summarizer_file = summarizer_arg

    s = match_cfg_file(summarizers_dir, [summarizer_file])[0]
    logger.info(f'Loading {s[0]}: {s[1]}')
    cfg = Config.fromfile(s[1])
    # Use summarizer_key to retrieve the summarizer definition
    # from the configuration file
    summarizer = cfg[summarizer_key]

    return Config(dict(models=models, datasets=datasets, summarizer=summarizer), format_python_code=False)


def get_config_type(obj) -> str:
    return f'{obj.__module__}.{obj.__name__}'


def fill_infer_cfg(cfg, args):
    new_cfg = dict(infer=dict(
        partitioner=dict(type=get_config_type(NumWorkerPartitioner),
                         num_worker=args.max_num_workers),
        runner=dict(
            max_num_workers=args.max_num_workers,
            debug=args.debug,
            task=dict(type=get_config_type(OpenICLInferTask)),
            lark_bot_url=cfg['lark_bot_url'],
        )), )
    if args.slurm:
        new_cfg['infer']['runner']['type'] = get_config_type(SlurmRunner)
        new_cfg['infer']['runner']['partition'] = args.partition
        new_cfg['infer']['runner']['quotatype'] = args.quotatype
        new_cfg['infer']['runner']['qos'] = args.qos
        new_cfg['infer']['runner']['retry'] = args.retry
    elif args.dlc:
        new_cfg['infer']['runner']['type'] = get_config_type(DLCRunner)
        new_cfg['infer']['runner']['aliyun_cfg'] = Config.fromfile(
            args.aliyun_cfg)
        new_cfg['infer']['runner']['retry'] = args.retry
    else:
        new_cfg['infer']['runner']['type'] = get_config_type(LocalRunner)
        new_cfg['infer']['runner'][
            'max_workers_per_gpu'] = args.max_workers_per_gpu
    cfg.merge_from_dict(new_cfg)
