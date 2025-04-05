import pytest
from unittest.mock import patch, MagicMock, ANY
import os
import sys
import torch

from ais_bench.benchmark.models.mindie_llm_api import MindieLLMAPI

@pytest.fixture
def mock_atb_speed_env():
    """模拟 ATB_SPEED_HOME_PATH 环境变量"""
    with patch.dict('os.environ', {'ATB_SPEED_HOME_PATH': '/fake/path'}):
        yield

@pytest.fixture
def mock_parunner():
    """模拟 PARunner 类"""
    with patch('examples.run_pa.PARunner') as mock_parunner_cls:
        mock_runner = MagicMock()
        mock_runner.model = MagicMock()
        mock_runner.model.dtype = torch.float16
        mock_runner.model.tokenizer = MagicMock()
        mock_runner.model.tokenizer.encode = MagicMock(return_value=[1,2,3])
        mock_parunner_cls.return_value = mock_runner
        yield mock_parunner_cls, mock_runner

@pytest.fixture
def mindie_model():
    """创建 MindieLLMAPI 实例的工厂"""
    def _create_model(**kwargs):
        default_kwargs = {
            'model_name': 'test_model',
            'data_type': 'fp16',
            'weight_dir': '/fake/model',
            'max_position_embedding': 4096,
            'decode_batch_size': 4,
            'prefill_batch_size': 0,
            'dp': 1,
            'tp': 1,
            'sp': 1,
            'moe_tp': 0,
            'pp': 1,
            'microbatch_size': 1,
            'moe_ep': 0,
            'trust_remote_code': True,
            'ignore_eos': False,
            'input_length': 256,
            'output_length': 512,
            'block_size': 64,
            'world_size': 2
        }
        return MindieLLMAPI(**{**default_kwargs, **kwargs})
    return _create_model

class TestMindieLLMAPI:

    def test_get_model_or_runner_normal(self, mock_atb_speed_env, mock_parunner, mindie_model):
        """测试正常初始化 PARunner"""
        mock_parunner_cls, mock_runner = mock_parunner
        model = mindie_model()

        # 验证路径修改
        assert '/fake/path' in sys.path[0]
        assert '/fake/path/../..' in sys.path[1]

        # 验证 PARunner 参数
        expected_args = {
            'rank': 0,
            'local_rank': 0,
            'world_size': 2,
            'max_prefill_tokens': -1,
            'block_size': 64,
            'model_path': '/fake/model',
            'max_position_embeddings': 768,  # 256+512
            'max_prefill_batch_size': 4,      # 使用 decode_batch_size 因为 prefill=0
            'max_batch_size': 4,
            'max_input_length': 256,
            'max_output_length': 512,
            'kw_args': '',
            'dp': 1,
            'tp': 1,
            'sp': 1,
            'moe_tp': 0,
            'pp': 1,
            'microbatch_size': 1,
            'moe_ep': 0,
            'trust_remote_code': True
        }
        mock_parunner_cls.assert_called_once_with(**expected_args)
        assert model.pa_runner == mock_runner

    def test_special_model_name(self, mock_parunner, mindie_model):
        """测试特殊模型名称处理"""
        # Qwen 系列模型应设置 max_position_embeddings=None
        for model_name in ["qwen2_72b", "qwen2_7b"]:
            model = mindie_model(model_name=model_name)
            call_args = mock_parunner[0].call_args[1]
            assert call_args['max_position_embeddings'] is None

    def test_dtype_mismatch(self, mock_parunner, mindie_model):
        """测试数据类型不匹配异常"""
        mock_runner = mock_parunner[1]
        mock_runner.model.dtype = torch.float32  # 与 data_type='fp16' 不匹配
        
        with pytest.raises(RuntimeError) as excinfo:
            mindie_model(data_type='fp16')
        
        assert "Inconsistent dtype" in str(excinfo.value)

    @patch('sys.path.insert')
    def test_missing_atb_speed_env(self, mock_path_insert, mindie_model):
        """测试缺少 ATB_SPEED_HOME_PATH 环境变量"""
        with pytest.raises(RuntimeError) as excinfo:
            mindie_model()  # 没有设置环境变量
            
        assert "Failed to import necessary packages" in str(excinfo.value)

    def test_generate_normal(self, mock_parunner, mindie_model):
        """测试生成文本流程"""
        model = mindie_model()
        mock_runner = mock_parunner[1]
        
        # 配置 mock 返回值
        expected_output = ["output1", "output2"]
        mock_runner.infer.return_value = (expected_output, None, None)
        
        # 执行生成
        inputs = ["test input 1", "test input 2"]
        results = model.generate(inputs, max_out_len=100)
        
        # 验证参数传递
        mock_runner.infer.assert_called_once_with(
            inputs,
            len(inputs),  # batch_size
            100,        # max_out_len
            False,       # ignore_eos
            False        # is_chat_model
        )
        assert results == expected_output

    def test_generate_with_chat_model(self, mock_parunner, mindie_model):
        """测试聊天模型生成"""
        model = mindie_model(is_chat_model=True)
        mock_runner = mock_parunner[1]
        mock_runner.infer.return_value = (["chat output"], None, None)
        
        results = model.generate(["user query"], max_out_len=50)
        
        mock_runner.infer.assert_called_once_with(
            ["user query"],
            1,    # batch_size
            50,    # max_out_len
            False, 
            True   # is_chat_model
        )

    def test_get_token_len(self, mock_parunner, mindie_model):
        """测试获取 token 长度"""
        model = mindie_model()
        model.tokenizer.encode.return_value = [1,2,3,4,5]
        
        assert model.get_token_len("test prompt") == 5
        model.tokenizer.encode.assert_called_once_with("test prompt")