import unittest
import os
import torch
from ptdbg_ascend.online_dispatch import utils

class TestUtils(unittest.TestCase):
    def setUp(self):
        self.data = torch.tensor([1, 2, 3])
        self.file_name = 'test_file'
        self.data_path = './'

    def test_get_callstack(self):
        result = utils.get_callstack()
        self.assertIsInstance(result, list)

    def test_np_save_data(self):
        utils.np_save_data(self.data, self.file_name, self.data_path)
        self.assertTrue(os.path.exists(os.path.join(self.data_path, f'{self.file_name}.npy')))

    def test_data_to_cpu(self):
        result = utils.data_to_cpu(self.data, 0, [])
        self.assertIsInstance(result, torch.Tensor)
        self.assertEqual(result.device, utils.cpu_device)

    def test_get_mp_logger(self):
        logger = utils.get_mp_logger()
        self.assertIsNotNone(logger)

    def test_logger_debug(self):
        utils.logger_debug('Test debug message')

    def test_logger_info(self):
        utils.logger_info('Test info message')

    def test_logger_warn(self):
        utils.logger_warn('Test warning message')

    def test_logger_error(self):
        utils.logger_error('Test error message')

    def test_logger_user(self):
        utils.logger_user('Test user message')

    def test_logger_logo(self):
        utils.logger_logo()

    def test_get_sys_info(self):
        result = utils.get_sys_info()
        self.assertIsInstance(result, str)

    def tearDown(self):
        if os.path.exists(os.path.join(self.data_path, f'{self.file_name}.npy')):
            os.remove(os.path.join(self.data_path, f'{self.file_name}.npy'))

