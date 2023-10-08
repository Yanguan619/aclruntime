import unittest
from unittest.mock import patch, MagicMock
from ptdbg_ascend.hook_module import register_hook

class TestRegisterHook(unittest.TestCase):

    def setUp(self):
        self.model = MagicMock()
        self.hook = MagicMock()

    def test_initialize_hook(self):
        register_hook.initialize_hook(self.hook)
        self.hook.assert_called_once()

    def test_add_clear_overflow(self):
        func = MagicMock()
        pid = 1234
        result = register_hook.add_clear_overflow(func, pid)
        self.assertIsInstance(result, type(func))

    def test_register_hook(self):
        with patch('ptdbg_ascend.hook_module.register_hook.register_hook_core') as mock_core:
            register_hook.register_hook(self.model, self.hook)
            mock_core.assert_called_once()

    def test_register_hook_core(self):
        with patch('ptdbg_ascend.hook_module.register_hook.initialize_hook') as mock_init:
            register_hook.register_hook_core(self.hook)
            mock_init.assert_called_once()

    def test_init_dump_config(self):
        kwargs = {'dump_mode': 'api', 'dump_config': 'config.json'}
        result = register_hook.init_dump_config(kwargs)
        self.assertEqual(result, ('api', 'config.json'))