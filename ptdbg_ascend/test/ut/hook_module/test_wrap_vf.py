import unittest
import torch
from ptdbg_ascend.hook_module import wrap_vf

class TestWrapVf(unittest.TestCase):
    def setUp(self):
        self.hook = lambda x: x

    def test_get_vf_ops(self):
        ops = wrap_vf.get_vf_ops()
        self.assertIsInstance(ops, list)

    def test_wrap_vf_op(self):
        wrapped_op = wrap_vf.wrap_vf_op('add', self.hook)
        result = wrapped_op(torch.tensor([1, 2]), torch.tensor([3, 4]))
        self.assertTrue(torch.equal(result, torch.tensor([4, 6])))

    def test_wrap_vf_ops_and_bind(self):
        wrap_vf.wrap_vf_ops_and_bind(self.hook)
        self.assertTrue(hasattr(wrap_vf.HOOKVfOP, 'wrap_add'))  # 假设'add'在你的_VF列表中

    def test_VfOPTemplate(self):
        vf_op_template = wrap_vf.VfOPTemplate('add', self.hook)
        result = vf_op_template(torch.tensor([1, 2]), torch.tensor([3, 4]))
        self.assertTrue(torch.equal(result, torch.tensor([4, 6])))