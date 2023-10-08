import unittest
import torch
from ptdbg_ascend.hook_module.wrap_npu_custom import HOOKNpuOP, NpuOPTemplate, wrap_npu_op, wrap_npu_ops_and_bind

class TestWrapNpuCustom(unittest.TestCase):

    def setUp(self):
        self.op_name = 'test_op'
        self.hook = lambda x: x
        self.args = (1, 2, 3)
        self.kwargs = {'a': 1, 'b': 2}

    def test_HOOKNpuOP(self):
        instance = HOOKNpuOP()
        self.assertIsInstance(instance, HOOKNpuOP)

    def test_NpuOPTemplate(self):
        instance = NpuOPTemplate(self.op_name, self.hook)
        self.assertEqual(instance.op_name_, self.op_name)
        self.assertEqual(instance.prefix_op_name_, "NPU_" + str(self.op_name) + "_")

    def test_wrap_npu_op(self):
        wrapped_op = wrap_npu_op(self.op_name, self.hook)
        result = wrapped_op(*self.args, **self.kwargs)
        self.assertIsInstance(result, NpuOPTemplate)

    def test_wrap_npu_ops_and_bind(self):
        wrap_npu_ops_and_bind(self.hook)
        for op_name in HOOKNpuOP.__dict__:
            if op_name.startswith('wrap_'):
                self.assertTrue(hasattr(HOOKNpuOP, op_name))