import unittest
import torch
import torch.nn as nn
from ptdbg_ascend.hook_module import HOOKModule

class TestHOOKModule(unittest.TestCase):
    def setUp(self):
        def hook(name):
            def hook_func(module, input, output):
                print(f"{name}: {output}")
            return hook_func

        self.module = HOOKModule(hook)

    def test_init(self):
        self.assertIsInstance(self.module, HOOKModule)
        self.assertFalse(self.module.has_overflow)
        self.assertEqual(self.module.input_args, tuple())
        self.assertEqual(self.module.input_kwargs, dict())

    def test_call(self):
        input_tensor = torch.tensor([1.0, 2.0, 3.0])
        output = self.module(input_tensor)
        self.assertEqual(output, input_tensor)

    def test_call_func(self):
        input_tensor = torch.tensor([1.0, 2.0, 3.0])
        output = self.module._call_func(input_tensor)
        self.assertEqual(output, input_tensor)