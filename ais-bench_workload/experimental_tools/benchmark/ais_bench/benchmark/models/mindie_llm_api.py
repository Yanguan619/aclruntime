import os
import sys
from typing import Dict, List, Optional, Union

import numpy as np
import torch
import transformers

from ais_bench.benchmark.models.base import BaseModel
from ais_bench.benchmark.models.base_api import APITemplateParser
from ais_bench.benchmark.registry import MODELS
from ais_bench.benchmark.utils.logging import get_logger
from ais_bench.benchmark.utils.prompt import PromptList


DTYPE_MAP = {"bf16": torch.bfloat16, "fp16": torch.float16}


@MODELS.register_module()
class MindieLLMAPI(BaseModel):
    """Model wrapper around HuggingFace models.

    Args:
        path (str): The name or path to HuggingFace's model.
        hf_cache_dir: Set the cache dir to HF model cache dir. If None, it will
            use the env variable HF_MODEL_HUB. Defaults to None.
        max_seq_len (int): The maximum length of the input sequence. Defaults
            to 2048.
        tokenizer_path (str): The path to the tokenizer. Defaults to None.
        tokenizer_kwargs (dict): Keyword arguments for the tokenizer.
            Defaults to {}.
        peft_path (str, optional): The name or path to the HuggingFace's PEFT
            model. If None, the original model will not be converted to PEFT.
            Defaults to None.
        tokenizer_only (bool): If True, only the tokenizer will be initialized.
            Defaults to False.
        model_kwargs (dict): Keyword arguments for the model, used in loader.
            Defaults to dict(device_map='auto').
        meta_template (Dict, optional): The model's meta prompt
            template if needed, in case the requirement of injecting or
            wrapping of any meta instructions.
        extract_pred_after_decode (bool): Whether to extract the prediction
            string from the decoded output string, instead of extract the
            prediction tokens before decoding. Defaults to False.
        batch_padding (bool): If False, inference with be performed in for-loop
            without batch padding.
        pad_token_id (int): The id of the padding token. Defaults to None. Use
            (#vocab + pad_token_id) if get negative value.
        mode (str, optional): The method of input truncation when input length
            exceeds max_seq_len. 'mid' represents the part of input to
            truncate. Defaults to 'none'.
        use_fastchat_template (str, optional): Whether to use fastchat to get
            the conversation template. If True, fastchat needs to be
            implemented first. Defaults to False.
        end_str (str, optional): Whether to trim generated strings with end_str
            if the model has special ending strings that are not handled well.
            Defaults to None.

    Note:
        About ``extract_pred_after_decode``: Commonly, we should extract the
        the prediction tokens before decoding. But for some tokenizers using
        ``sentencepiece``, like LLaMA,  this behavior may change the number of
        whitespaces, which is harmful for Python programming tasks.
    """

    def __init__(self, **kwargs):

        self.rank = int(os.getenv("RANK", "0"))
        self.local_rank = int(os.getenv("LOCAL_RANK", "0"))
        self.world_size = kwargs.get('world_size')
        self.block_size = kwargs.get('block_size')

        self.model_name = kwargs.get('model_name')
        self.data_type = kwargs.get('data_type')  # fp16 / bf16
        self.weight_dir = kwargs.get('weight_dir')
        self.max_position_embedding = kwargs.get('max_position_embedding')
        self.is_chat_model = kwargs.get('is_chat_model')
        self.prefill_batch_size = kwargs.get('prefill_batch_size')
        self.kw_args = ''
        self.dp = kwargs.get('dp')
        self.tp = kwargs.get('tp')
        self.sp = kwargs.get('sp')
        self.moe_tp = kwargs.get('moe_tp')
        self.pp = kwargs.get('pp')
        self.microbatch_size = kwargs.get('microbatch_size')
        self.moe_ep = kwargs.get('moe_ep')
        self.trust_remote_code = kwargs.get('trust_remote_code')
        self.ignore_eos = kwargs.get('ignore_eos')
        self.input_length = kwargs.get('input_length')
        self.output_length = kwargs.get('output_length')
        self.decode_batch_size = kwargs.get('decode_batch_size')
        self.logger = get_logger()
        self.pa_runner = None

        self.prepare_environ()
        self.__get_model_or_runner(self.input_length, self.output_length)
        if self.pa_runner == None:
            raise RuntimeError("Model loading failed")
        self.pa_runner.warm_up()

        super().__init__(path=self.weight_dir,
                         max_seq_len=self.output_length,
                         tokenizer_only=False,
                         meta_template=None)

    def prepare_environ(self):
        os.environ['ATB_LAYER_INTERNAL_TENSOR_REUSE'] = "1"
        os.environ['INF_NAN_MODE_ENABLE'] = "0"
        os.environ['ATB_OPERATION_EXECUTE_ASYNC'] = "1"
        os.environ['ATB_CONVERT_NCHW_TO_ND'] = "1"
        os.environ['TASK_QUEUE_ENABLE'] = "1"
        os.environ['ATB_WORKSPACE_MEM_ALLOC_GLOBAL'] = "1"
        os.environ['ATB_CONTEXT_WORKSPACE_SIZE'] = "0"
        os.environ['ATB_LAUNCH_KERNEL_WITH_TILING'] = "1"

    def __get_model_or_runner(self, input_length, output_length, warmup_bs=0):

        try:
            ATB_SPEED_HOME_PATH = os.environ.get("ATB_SPEED_HOME_PATH")
            if ATB_SPEED_HOME_PATH not in sys.path:
                sys.path.insert(0, os.path.join(ATB_SPEED_HOME_PATH, "../.."))
                sys.path.insert(0, ATB_SPEED_HOME_PATH)
            from atb_llm.utils.env import ENV
            from examples.run_pa import PARunner
        except Exception:
            raise RuntimeError("Failed to import necessary packages")

        rank = "rank"
        world_size = "world_size"
        local_rank = "local_rank"
        model_path = "model_path"
        max_position_embeddings = "max_position_embeddings"
        max_input_length = "max_input_length"
        max_output_length = "max_output_length"
        trust_remote_code = "trust_remote_code"


        prefill_batch_size = self.decode_batch_size if self.prefill_batch_size == 0 else self.prefill_batch_size

        input_dict = {
            rank: self.rank,
            local_rank: self.local_rank,
            world_size: self.world_size,
            'max_prefill_tokens': -1,
            'block_size': self.block_size,
            model_path: self.weight_dir,
            max_position_embeddings: (self.max_position_embedding
                                        if self.max_position_embedding != -1
                                        else input_length + output_length),
            'max_prefill_batch_size': prefill_batch_size,
            'max_batch_size': warmup_bs if warmup_bs != 0 else self.decode_batch_size,
            max_input_length: input_length,
            max_output_length: output_length,
            'kw_args': self.kw_args,
            'dp': self.dp,
            'tp': self.tp,
            'sp': self.sp,
            'moe_tp': self.moe_tp,
            'pp': self.pp,
            'microbatch_size': self.microbatch_size,
            'moe_ep': self.moe_ep,
            trust_remote_code: self.trust_remote_code
        }
        if self.model_name == "qwen2_72b" or self.model_name == "qwen2_7b":
            input_dict[max_position_embeddings] = None
        self.pa_runner = PARunner(**input_dict)
        model_dtype = self.pa_runner.model.dtype
        self.tokenizer = self.pa_runner.model.tokenizer
        user_dtype = DTYPE_MAP.get(self.data_type, None)
        if user_dtype != model_dtype:
            self.logger.error(
                "Inconsistent dtype: Input dtype: %s, model weight dtype: %s. please check",
                user_dtype, model_dtype)
            raise RuntimeError(
                f"Inconsistent dtype: Input dtype: {user_dtype}, " +
                f"model weight dtype: {model_dtype}. please check")

        self.logger.info('%d pa_runner: %s', self.rank, self.pa_runner)


    def generate(self,
                 inputs: List[str],
                 max_out_len: int,
                 **kwargs) -> List[str]:
        """Generate results given a list of inputs.

        Args:
            inputs (List[str]): A list of strings.
            max_out_len (int): The maximum length of the output.

        Returns:
            List[str]: A list of generated strings.
        """
        with torch.no_grad():
            generate_texts, _, _ = self.pa_runner.infer(inputs,
                                                        len(inputs),
                                                        max_out_len,
                                                        self.ignore_eos,
                                                        self.is_chat_model)
            
        return generate_texts


    def get_token_len(self, prompt: str) -> int:
        """Get lengths of the tokenized strings.

        Args:
            prompt (str): Input string.

        Returns:
            int: Length of the input tokens
        """
        return len(self.tokenizer.encode(prompt))

