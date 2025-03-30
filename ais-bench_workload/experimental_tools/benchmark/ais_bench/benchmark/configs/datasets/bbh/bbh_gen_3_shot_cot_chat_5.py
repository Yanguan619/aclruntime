import os
from mmengine.config import read_base
from ais_bench.benchmark.openicl.icl_prompt_template import PromptTemplate
from ais_bench.benchmark.openicl.icl_retriever import ZeroRetriever
from ais_bench.benchmark.openicl.icl_inferencer import GenInferencer
from ais_bench.benchmark.datasets import BBHDataset, bbh_mcq_postprocess, BBHEvaluator, BBHEvaluator_mcq

with read_base():
    from .bbh_subset_settings import settings

bbh_datasets = []
for name, test_type in settings:
    with open(os.path.join("ais_bench/datasets/BBH", 'lib_prompt', f'{name}.txt'), 'r') as f:
        hint = f.read()

    task_prompt, body = hint.split('\n\nQ:', 1)
    sections = ('Q:' + body).split('\n\n')
    prompt_rounds = []
    for index, section in enumerate(sections):
        question, answer = section.split('\nA:')
        answer = 'A:' + answer
        if index == 0:
            desc = task_prompt.strip() + '\n'
        else:
            desc = ''
        prompt_rounds.append(dict(role='HUMAN', prompt=f'{desc}{question.strip()}'))
        prompt_rounds.append(dict(role='BOT', prompt=answer.strip()))
    prompt_rounds.append(dict(role='HUMAN', prompt='Q: {input}'))

    bbh_reader_cfg = dict(input_columns=['input'], output_column='target')

    bbh_infer_cfg = dict(
        prompt_template=dict(type=PromptTemplate, template=dict(round=prompt_rounds)),
        retriever=dict(type=ZeroRetriever),
        inferencer=dict(type=GenInferencer))

    if test_type == 'mcq':
        bbh_eval_cfg = dict(
            evaluator=dict(type=BBHEvaluator_mcq),
            pred_role='BOT',
            pred_postprocessor=dict(type=bbh_mcq_postprocess),
            dataset_postprocessor=dict(type=bbh_mcq_postprocess))
    else:
        bbh_eval_cfg = dict(
            evaluator=dict(type=BBHEvaluator),
            pred_role='BOT')

    bbh_datasets.append(
        dict(
            type=BBHDataset,
            path='ais_bench/datasets/BBH/data',
            name=name,
            abbr='bbh-' + name,
            reader_cfg=bbh_reader_cfg.copy(),
            infer_cfg=bbh_infer_cfg.copy(),
            eval_cfg=bbh_eval_cfg.copy()))
