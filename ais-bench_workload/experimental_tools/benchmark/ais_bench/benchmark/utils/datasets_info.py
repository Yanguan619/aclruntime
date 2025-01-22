DATASETS_MAPPING = {
    # ADVGLUE Datasets
    "opencompas1s/advglue-dev": {
        "ms_id": None,
        "hf_id": None,
        "local": "./data/adv_glue/dev_ann.json",
    },
    # AGIEval Datasets
    "opencompas1s/agieval": {
        "ms_id": "opencompas1s/agieval",
        "hf_id": "opencompas1s/agieval",
        "local": "./data/AGIEval/data/v1/",
    },
    # ARC Datasets(Test)
    "opencompas1s/ai2_arc-test": {
        "ms_id": "opencompas1s/ai2_arc",
        "hf_id": "opencompas1s/ai2_arc",
        "local": "./data/ARC/ARC-c/ARC-Challenge-Test.jsonl",
    },
    "opencompas1s/ai2_arc-dev": {
        "ms_id": "opencompas1s/ai2_arc",
        "hf_id": "opencompas1s/ai2_arc",
        "local": "./data/ARC/ARC-c/ARC-Challenge-Dev.jsonl",
    },
    "opencompas1s/ai2_arc-easy-dev": {
        "ms_id": "opencompas1s/ai2_arc",
        "hf_id": "opencompas1s/ai2_arc",
        "local": "./data/ARC/ARC-e/ARC-Easy-Dev.jsonl",
    },
    # BBH
    "opencompas1s/bbh": {
        "ms_id": "opencompas1s/bbh",
        "hf_id": "opencompas1s/bbh",
        "local": "./data/BBH/data",
    },
    # C-Eval
    "opencompas1s/ceval-exam": {
        "ms_id": "opencompas1s/ceval-exam",
        "hf_id": "opencompas1s/ceval-exam",
        "local": "./data/ceval/formal_ceval",
    },
    # AFQMC
    "opencompas1s/afqmc-dev": {
        "ms_id": "opencompas1s/afqmc",
        "hf_id": "opencompas1s/afqmc",
        "local": "./data/CLUE/AFQMC/dev.json",
    },
    # CMNLI
    "opencompas1s/cmnli-dev": {
        "ms_id": "opencompas1s/cmnli",
        "hf_id": "opencompas1s/cmnli",
        "local": "./data/CLUE/cmnli/cmnli_public/dev.json",
    },
    # OCNLI
    "opencompas1s/OCNLI-dev": {
        "ms_id": "opencompas1s/OCNLI",
        "hf_id": "opencompas1s/OCNLI",
        "local": "./data/CLUE/OCNLI/dev.json",
    },
    # ChemBench
    "opencompas1s/ChemBench": {
        "ms_id": "opencompas1s/ChemBench",
        "hf_id": "opencompas1s/ChemBench",
        "local": "./data/ChemBench/",
    },
    # CMMLU
    "opencompas1s/cmmlu": {
        "ms_id": "opencompas1s/cmmlu",
        "hf_id": "opencompas1s/cmmlu",
        "local": "./data/cmmlu/",
    },
    # CommonsenseQA
    "opencompas1s/commonsense_qa": {
        "ms_id": "opencompas1s/commonsense_qa",
        "hf_id": "opencompas1s/commonsense_qa",
        "local": "./data/commonsenseqa",
    },
    # CMRC
    "opencompas1s/cmrc_dev": {
        "ms_id": "opencompas1s/cmrc_dev",
        "hf_id": "opencompas1s/cmrc_dev",
        "local": "./data/CLUE/CMRC/dev.json",
    },
    # DRCD_dev
    "opencompas1s/drcd_dev": {
        "ms_id": "opencompas1s/drcd_dev",
        "hf_id": "opencompas1s/drcd_dev",
        "local": "./data/CLUE/DRCD/dev.json",
    },
    # clozeTest_maxmin
    "opencompas1s/clozeTest_maxmin": {
        "ms_id": None,
        "hf_id": None,
        "local": "./data/clozeTest-maxmin/python/clozeTest.json",
    },
    # clozeTest_maxmin
    "opencompas1s/clozeTest_maxmin_answers": {
        "ms_id": None,
        "hf_id": None,
        "local": "./data/clozeTest-maxmin/python/answers.txt",
    },
    # Flores
    "opencompas1s/flores": {
        "ms_id": "opencompas1s/flores",
        "hf_id": "opencompas1s/flores",
        "local": "./data/flores_first100",
    },
    # MBPP
    "opencompas1s/mbpp": {
        "ms_id": "opencompas1s/mbpp",
        "hf_id": "opencompas1s/mbpp",
        "local": "./data/mbpp/mbpp.jsonl",
    },
    # 'opencompas1s/mbpp': {
    #     'ms_id': 'opencompas1s/mbpp',
    #     'hf_id': 'opencompas1s/mbpp',
    #     'local': './data/mbpp/mbpp.jsonl',
    # },
    "opencompas1s/sanitized_mbpp": {
        "ms_id": "opencompas1s/mbpp",
        "hf_id": "opencompas1s/mbpp",
        "local": "./data/mbpp/sanitized-mbpp.jsonl",
    },
    # GSM
    "opencompas1s/gsm8k": {
        "ms_id": "opencompas1s/gsm8k",
        "hf_id": "opencompas1s/gsm8k",
        "local": "./data/gsm8k/",
    },
    # HellaSwag
    "opencompas1s/hellaswag": {
        "ms_id": "opencompas1s/hellaswag",
        "hf_id": "opencompas1s/hellaswag",
        "local": "./data/hellaswag/hellaswag.jsonl",
    },
    # HellaSwagICE
    "opencompas1s/hellaswag_ice": {
        "ms_id": "opencompas1s/hellaswag",
        "hf_id": "opencompas1s/hellaswag",
        "local": "./data/hellaswag/",
    },
    # HumanEval
    "opencompas1s/humaneval": {
        "ms_id": "opencompas1s/humaneval",
        "hf_id": "opencompas1s/humaneval",
        "local": "./data/humaneval/human-eval-v2-20210705.jsonl",
    },
    # HumanEvalCN
    "opencompas1s/humaneval_cn": {
        "ms_id": "opencompas1s/humaneval",
        "hf_id": "opencompas1s/humaneval",
        "local": "./data/humaneval_cn/human-eval-cn-v2-20210705.jsonl",
    },
    #KORBENCH
    "opencompas1s/korbench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/korbench",
    },
    # Lambada
    "opencompas1s/lambada": {
        "ms_id": "opencompas1s/lambada",
        "hf_id": "opencompas1s/lambada",
        "local": "./data/lambada/test.jsonl",
    },
    # LCSTS
    "opencompas1s/LCSTS": {
        "ms_id": "opencompas1s/LCSTS",
        "hf_id": "opencompas1s/LCSTS",
        "local": "./data/LCSTS",
    },
    # MATH
    "opencompas1s/math": {
        "ms_id": "opencompas1s/math",
        "hf_id": "opencompas1s/math",
        "local": "./data/math/",
    },
    # MMLU
    "opencompas1s/mmlu": {
        "ms_id": "opencompas1s/mmlu",
        "hf_id": "opencompas1s/mmlu",
        "local": "./data/mmlu/",
    },
    # MMLU_PRO
    "opencompas1s/mmlu_pro": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/mmlu_pro",
    },
    # NQ
    "opencompas1s/natural_question": {
        "ms_id": "opencompas1s/natural_question",
        "hf_id": "opencompas1s/natural_question",
        "local": "./data/nq/",
    },
    # OpenBook QA-test
    "opencompas1s/openbookqa_test": {
        "ms_id": "opencompas1s/openbookqa",
        "hf_id": "opencompas1s/openbookqa",
        "local": "./data/openbookqa/Main/test.jsonl",
    },
    # OpenBook QA-fact
    "opencompas1s/openbookqa_fact": {
        "ms_id": "opencompas1s/openbookqa",
        "hf_id": "opencompas1s/openbookqa",
        "local": "./data/openbookqa/Additional/test_complete.jsonl",
    },
    # PIQA
    "opencompas1s/piqa": {
        "ms_id": "opencompas1s/piqa",
        "hf_id": "opencompas1s/piqa",
        "local": "./data/piqa",
    },
    # RACE
    "opencompas1s/race": {
        "ms_id": "opencompas1s/race",
        "hf_id": "opencompas1s/race",
        "local": "./data/race/",
    },
    # SIQA
    "opencompas1s/siqa": {
        "ms_id": "opencompas1s/siqa",
        "hf_id": "opencompas1s/siqa",
        "local": "./data/siqa",
    },
    # XStoryCloze
    "opencompas1s/xstory_cloze": {
        "ms_id": "opencompas1s/xstory_cloze",
        "hf_id": "opencompas1s/xstory_cloze",
        "local": "./data/xstory_cloze",
    },
    # StrategyQA
    "opencompas1s/strategy_qa": {
        "ms_id": "opencompas1s/strategy_qa",
        "hf_id": "opencompas1s/strategy_qa",
        "local": "./data/strategyqa/strategyQA_train.json",
    },
    # SummEdits
    "opencompas1s/summedits": {
        "ms_id": "opencompas1s/summedits",
        "hf_id": "opencompas1s/summedits",
        "local": "./data/summedits/summedits.jsonl",
    },
    # SuperGLUE
    "opencompas1s/boolq": {
        "ms_id": "opencompas1s/boolq",
        "hf_id": "opencompas1s/boolq",
        "local": "./data/SuperGLUE/BoolQ/val.jsonl",
    },
    # TriviaQA
    "opencompas1s/trivia_qa": {
        "ms_id": "opencompas1s/trivia_qa",
        "hf_id": "opencompas1s/trivia_qa",
        "local": "./data/triviaqa/",
    },
    # TydiQA
    "opencompas1s/tydiqa": {
        "ms_id": "opencompas1s/tydiqa",
        "hf_id": "opencompas1s/tydiqa",
        "local": "./data/tydiqa/",
    },
    # Winogrande
    "opencompas1s/winogrande": {
        "ms_id": "opencompas1s/winogrande",
        "hf_id": "opencompas1s/winogrande",
        "local": "./data/winogrande/",
    },
    # XSum
    "opencompas1s/xsum": {
        "ms_id": "opencompas1s/xsum",
        "hf_id": "opencompas1s/xsum",
        "local": "./data/Xsum/dev.jsonl",
    },
    # Longbench
    "opencompas1s/Longbench": {
        "ms_id": "",
        "hf_id": "THUDM/LongBench",
        "local": "./data/Longbench",
    },
    # Needlebench
    "opencompas1s/needlebench": {
        "ms_id": "",
        "hf_id": "opencompas1s/needlebench",
        "local": "./data/needlebench",
    },
    "opencompas1s/code_generation_lite": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/code_generation_lite",
    },
    "opencompas1s/execution-v2": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/execution-v2",
    },
    "opencompas1s/test_generation": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/test_generation",
    },
    "opencompas1s/aime2024": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/aime.jsonl",
    },
    "opencompas1s/cmo_fib": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/cmo.jsonl",
    },
    "opencompas1s/nq_open": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/nq-open/",
    },
    "opencompas1s/GAOKAO-BENCH": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/GAOKAO-BENCH/data",
    },
    "opencompas1s/WikiBench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/WikiBench/",
    },
    "opencompas1s/mmmlu_lite": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/mmmlu_lite",
    },
    "opencompas1s/mmmlu_lite": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/mmmlu_lite",
    },
    "opencompas1s/musr": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/musr",
    },
    "opencompas1s/babilong": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/babilong/data/",
    },
    "P-MMEval": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/P-MMEval/",
    },
    "opencompas1s/arc_prize_public_evaluation": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/arc_prize_public_evaluation",
    },
    "opencompas1s/simpleqa": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/simpleqa/simple_qa_test_set.csv",
    },
    "opencompas1s/chinese_simpleqa": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/chinese_simpleqa",
    },
    "opencompas1s/LiveMathBench202412": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/LiveMathBench/",
    },
    "opencompas1s/LiveMathBench": {
        "ms_id": "",
        "hf_id": "opencompas1s/LiveMathBench",
        "local": "./data/LiveMathBench/",
    },
    "opencompas1s/LiveReasonBench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/LiveReasonBench/",
    },
    "opencompas1s/bigcodebench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/bigcodebench/",
    },
    "opencompas1s/qabench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/qabench",
    },
    "opencompas1s/livestembench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/livestembench/",
    },
    "opencompas1s/longbenchv2": {
        "ms_id": "",
        "hf_id": "THUDM/LongBench-v2",
        "local": "./data/longbenchv2/data.json",
    },
}

DATASETS_URL = {}
