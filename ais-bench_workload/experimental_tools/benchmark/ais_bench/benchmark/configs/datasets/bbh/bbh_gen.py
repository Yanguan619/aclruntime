from mmengine.config import read_base

with read_base():
    from .bbh_gen_3_shot_cot_chat_5 import bbh_datasets  # noqa: F401, F403
