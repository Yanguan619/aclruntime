DATASETS_MAPPING = {
    # ADVGLUE Datasets
    "opencompass/advglue-dev": {
        "ms_id": None,
        "hf_id": None,
        "local": "./data/adv_glue/dev_ann.json",
    },
    # AGIEval Datasets
    "opencompass/agieval": {
        "ms_id": "opencompass/agieval",
        "hf_id": "opencompass/agieval",
        "local": "./data/AGIEval/data/v1/",
    },
    # ARC Datasets(Test)
    "opencompass/ai2_arc-test": {
        "ms_id": "opencompass/ai2_arc",
        "hf_id": "opencompass/ai2_arc",
        "local": "./data/ARC/ARC-c/ARC-Challenge-Test.jsonl",
    },
    "opencompass/ai2_arc-dev": {
        "ms_id": "opencompass/ai2_arc",
        "hf_id": "opencompass/ai2_arc",
        "local": "./data/ARC/ARC-c/ARC-Challenge-Dev.jsonl",
    },
    "opencompass/ai2_arc-easy-dev": {
        "ms_id": "opencompass/ai2_arc",
        "hf_id": "opencompass/ai2_arc",
        "local": "./data/ARC/ARC-e/ARC-Easy-Dev.jsonl",
    },
    # BBH
    "opencompass/bbh": {
        "ms_id": "opencompass/bbh",
        "hf_id": "opencompass/bbh",
        "local": "./data/BBH/data",
    },
    # C-Eval
    "opencompass/ceval-exam": {
        "ms_id": "opencompass/ceval-exam",
        "hf_id": "opencompass/ceval-exam",
        "local": "./data/ceval/formal_ceval",
    },
    # AFQMC
    "opencompass/afqmc-dev": {
        "ms_id": "opencompass/afqmc",
        "hf_id": "opencompass/afqmc",
        "local": "./data/CLUE/AFQMC/dev.json",
    },
    # CMNLI
    "opencompass/cmnli-dev": {
        "ms_id": "opencompass/cmnli",
        "hf_id": "opencompass/cmnli",
        "local": "./data/CLUE/cmnli/cmnli_public/dev.json",
    },
    # OCNLI
    "opencompass/OCNLI-dev": {
        "ms_id": "opencompass/OCNLI",
        "hf_id": "opencompass/OCNLI",
        "local": "./data/CLUE/OCNLI/dev.json",
    },
    # ChemBench
    "opencompass/ChemBench": {
        "ms_id": "opencompass/ChemBench",
        "hf_id": "opencompass/ChemBench",
        "local": "./data/ChemBench/",
    },
    # CMMLU
    "opencompass/cmmlu": {
        "ms_id": "opencompass/cmmlu",
        "hf_id": "opencompass/cmmlu",
        "local": "./data/cmmlu/",
    },
    # CommonsenseQA
    "opencompass/commonsense_qa": {
        "ms_id": "opencompass/commonsense_qa",
        "hf_id": "opencompass/commonsense_qa",
        "local": "./data/commonsenseqa",
    },
    # CMRC
    "opencompass/cmrc_dev": {
        "ms_id": "opencompass/cmrc_dev",
        "hf_id": "opencompass/cmrc_dev",
        "local": "./data/CLUE/CMRC/dev.json",
    },
    # DRCD_dev
    "opencompass/drcd_dev": {
        "ms_id": "opencompass/drcd_dev",
        "hf_id": "opencompass/drcd_dev",
        "local": "./data/CLUE/DRCD/dev.json",
    },
    # clozeTest_maxmin
    "opencompass/clozeTest_maxmin": {
        "ms_id": None,
        "hf_id": None,
        "local": "./data/clozeTest-maxmin/python/clozeTest.json",
    },
    # clozeTest_maxmin
    "opencompass/clozeTest_maxmin_answers": {
        "ms_id": None,
        "hf_id": None,
        "local": "./data/clozeTest-maxmin/python/answers.txt",
    },
    # Flores
    "opencompass/flores": {
        "ms_id": "opencompass/flores",
        "hf_id": "opencompass/flores",
        "local": "./data/flores_first100",
    },
    # MBPP
    "opencompass/mbpp": {
        "ms_id": "opencompass/mbpp",
        "hf_id": "opencompass/mbpp",
        "local": "./data/mbpp/mbpp.jsonl",
    },
    # 'opencompass/mbpp': {
    #     'ms_id': 'opencompass/mbpp',
    #     'hf_id': 'opencompass/mbpp',
    #     'local': './data/mbpp/mbpp.jsonl',
    # },
    "opencompass/sanitized_mbpp": {
        "ms_id": "opencompass/mbpp",
        "hf_id": "opencompass/mbpp",
        "local": "./data/mbpp/sanitized-mbpp.jsonl",
    },
    # GSM
    "opencompass/gsm8k": {
        "ms_id": "opencompass/gsm8k",
        "hf_id": "opencompass/gsm8k",
        "local": "./data/gsm8k/",
    },
    # HellaSwag
    "opencompass/hellaswag": {
        "ms_id": "opencompass/hellaswag",
        "hf_id": "opencompass/hellaswag",
        "local": "./data/hellaswag/hellaswag.jsonl",
    },
    # HellaSwagICE
    "opencompass/hellaswag_ice": {
        "ms_id": "opencompass/hellaswag",
        "hf_id": "opencompass/hellaswag",
        "local": "./data/hellaswag/",
    },
    # HumanEval
    "opencompass/humaneval": {
        "ms_id": "opencompass/humaneval",
        "hf_id": "opencompass/humaneval",
        "local": "./data/humaneval/human-eval-v2-20210705.jsonl",
    },
    # HumanEvalCN
    "opencompass/humaneval_cn": {
        "ms_id": "opencompass/humaneval",
        "hf_id": "opencompass/humaneval",
        "local": "./data/humaneval_cn/human-eval-cn-v2-20210705.jsonl",
    },
    #KORBENCH
    "opencompass/korbench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/korbench",
    },
    # Lambada
    "opencompass/lambada": {
        "ms_id": "opencompass/lambada",
        "hf_id": "opencompass/lambada",
        "local": "./data/lambada/test.jsonl",
    },
    # LCSTS
    "opencompass/LCSTS": {
        "ms_id": "opencompass/LCSTS",
        "hf_id": "opencompass/LCSTS",
        "local": "./data/LCSTS",
    },
    # MATH
    "opencompass/math": {
        "ms_id": "opencompass/math",
        "hf_id": "opencompass/math",
        "local": "./data/math/",
    },
    # MMLU
    "opencompass/mmlu": {
        "ms_id": "opencompass/mmlu",
        "hf_id": "opencompass/mmlu",
        "local": "./data/mmlu/",
    },
    # MMLU_PRO
    "opencompass/mmlu_pro": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/mmlu_pro",
    },
    # NQ
    "opencompass/natural_question": {
        "ms_id": "opencompass/natural_question",
        "hf_id": "opencompass/natural_question",
        "local": "./data/nq/",
    },
    # OpenBook QA-test
    "opencompass/openbookqa_test": {
        "ms_id": "opencompass/openbookqa",
        "hf_id": "opencompass/openbookqa",
        "local": "./data/openbookqa/Main/test.jsonl",
    },
    # OpenBook QA-fact
    "opencompass/openbookqa_fact": {
        "ms_id": "opencompass/openbookqa",
        "hf_id": "opencompass/openbookqa",
        "local": "./data/openbookqa/Additional/test_complete.jsonl",
    },
    # PIQA
    "opencompass/piqa": {
        "ms_id": "opencompass/piqa",
        "hf_id": "opencompass/piqa",
        "local": "./data/piqa",
    },
    # RACE
    "opencompass/race": {
        "ms_id": "opencompass/race",
        "hf_id": "opencompass/race",
        "local": "./data/race/",
    },
    # SIQA
    "opencompass/siqa": {
        "ms_id": "opencompass/siqa",
        "hf_id": "opencompass/siqa",
        "local": "./data/siqa",
    },
    # XStoryCloze
    "opencompass/xstory_cloze": {
        "ms_id": "opencompass/xstory_cloze",
        "hf_id": "opencompass/xstory_cloze",
        "local": "./data/xstory_cloze",
    },
    # StrategyQA
    "opencompass/strategy_qa": {
        "ms_id": "opencompass/strategy_qa",
        "hf_id": "opencompass/strategy_qa",
        "local": "./data/strategyqa/strategyQA_train.json",
    },
    # SummEdits
    "opencompass/summedits": {
        "ms_id": "opencompass/summedits",
        "hf_id": "opencompass/summedits",
        "local": "./data/summedits/summedits.jsonl",
    },
    # SuperGLUE
    "opencompass/boolq": {
        "ms_id": "opencompass/boolq",
        "hf_id": "opencompass/boolq",
        "local": "./data/SuperGLUE/BoolQ/val.jsonl",
    },
    # TriviaQA
    "opencompass/trivia_qa": {
        "ms_id": "opencompass/trivia_qa",
        "hf_id": "opencompass/trivia_qa",
        "local": "./data/triviaqa/",
    },
    # TydiQA
    "opencompass/tydiqa": {
        "ms_id": "opencompass/tydiqa",
        "hf_id": "opencompass/tydiqa",
        "local": "./data/tydiqa/",
    },
    # Winogrande
    "opencompass/winogrande": {
        "ms_id": "opencompass/winogrande",
        "hf_id": "opencompass/winogrande",
        "local": "./data/winogrande/",
    },
    # XSum
    "opencompass/xsum": {
        "ms_id": "opencompass/xsum",
        "hf_id": "opencompass/xsum",
        "local": "./data/Xsum/dev.jsonl",
    },
    # Longbench
    "opencompass/Longbench": {
        "ms_id": "",
        "hf_id": "THUDM/LongBench",
        "local": "./data/Longbench",
    },
    # Needlebench
    "opencompass/needlebench": {
        "ms_id": "",
        "hf_id": "opencompass/needlebench",
        "local": "./data/needlebench",
    },
    "opencompass/code_generation_lite": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/code_generation_lite",
    },
    "opencompass/execution-v2": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/execution-v2",
    },
    "opencompass/test_generation": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/test_generation",
    },
    "opencompass/aime2024": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/aime.jsonl",
    },
    "opencompass/cmo_fib": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/cmo.jsonl",
    },
    "opencompass/nq_open": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/nq-open/",
    },
    "opencompass/GAOKAO-BENCH": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/GAOKAO-BENCH/data",
    },
    "opencompass/WikiBench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/WikiBench/",
    },
    "opencompass/mmmlu_lite": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/mmmlu_lite",
    },
    "opencompass/mmmlu_lite": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/mmmlu_lite",
    },
    "opencompass/musr": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/musr",
    },
    "opencompass/babilong": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/babilong/data/",
    },
    "P-MMEval": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/P-MMEval/",
    },
    "opencompass/arc_prize_public_evaluation": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/arc_prize_public_evaluation",
    },
    "opencompass/simpleqa": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/simpleqa/simple_qa_test_set.csv",
    },
    "opencompass/chinese_simpleqa": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/chinese_simpleqa",
    },
    "opencompass/LiveMathBench202412": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/LiveMathBench/",
    },
    "opencompass/LiveMathBench": {
        "ms_id": "",
        "hf_id": "opencompass/LiveMathBench",
        "local": "./data/LiveMathBench/",
    },
    "opencompass/LiveReasonBench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/LiveReasonBench/",
    },
    "opencompass/bigcodebench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/bigcodebench/",
    },
    "opencompass/qabench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/qabench",
    },
    "opencompass/livestembench": {
        "ms_id": "",
        "hf_id": "",
        "local": "./data/livestembench/",
    },
    "opencompass/longbenchv2": {
        "ms_id": "",
        "hf_id": "THUDM/LongBench-v2",
        "local": "./data/longbenchv2/data.json",
    },
}

DATASETS_URL = {}
