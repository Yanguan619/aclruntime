import unittest
import torch
from ptdbg_ascend.hook_module import wrap_functional as wf

class TestWrapFunctional(unittest.TestCase):

    def test_remove_dropout(self):
        input_tensor = torch.randn(20, 16)
        wf.remove_dropout()
        output_tensor = torch.nn.functional.dropout(input_tensor)
        self.assertTrue(torch.equal(input_tensor, output_tensor))

    def test_get_functional_ops(self):
        expected_ops = {'relu', 'sigmoid', 'softmax'}
        actual_ops = wf.get_functional_ops()
        self.assertTrue(expected_ops.issubset(actual_ops))

    def test_FunctionalOPTemplate(self):
        instance = wf.FunctionalOPTemplate('relu', None)
        input_tensor = torch.randn(20, 16)
        expected_output = torch.relu(input_tensor)
        actual_output = instance.forward(input_tensor)
        self.assertTrue(torch.equal(expected_output, actual_output))

    def test_wrap_functional_op(self):
        wrapped_op = wf.wrap_functional_op('relu', None)
        input_tensor = torch.randn(20, 16)
        expected_output = torch.relu(input_tensor)
        actual_output = wrapped_op(input_tensor)
        self.assertTrue(torch.equal(expected_output, actual_output))

    def test_wrap_functional_ops_and_bind(self):
        wf.wrap_functional_ops_and_bind(None)
        self.assertTrue(hasattr(wf.HOOKFunctionalOP, 'wrap_relu'))