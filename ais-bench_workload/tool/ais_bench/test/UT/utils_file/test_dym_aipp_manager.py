import unittest
from unittest.mock import MagicMock, patch
from ais_bench.infer.dym_aipp_manager import DymAippManager, ConfigParser, logger, AIPP_HEAD_STR 

class TestDymAippManager(unittest.TestCase):
    def setUp(self):
        # Mock session and config file
        self.session = MagicMock()
        self.config_file = "path/to/config.ini"
        self.batchsize = 32

        # Create DymAippManager instance
        self.manager = DymAippManager(self.session, self.config_file, self.batchsize)

        # Mock ConfigParser
        self.manager.cfg = MagicMock()
        self.manager.cfg.sections.return_value = [AIPP_HEAD_STR]
        self.manager.cfg.options.return_value = [
            'input_format', 'src_image_size_w', 'src_image_size_h', 'csc_switch', 'matrix_r0c0', 'matrix_r0c1',
            'matrix_r0c2', 'matrix_r1c0', 'matrix_r1c1', 'matrix_r1c2', 'matrix_r2c0', 'matrix_r2c1', 'matrix_r2c2',
            'output_bias_0', 'output_bias_1', 'output_bias_2', 'input_bias_0', 'input_bias_1', 'input_bias_2',
            'crop', 'load_start_pos_w', 'load_start_pos_h', 'crop_size_w', 'crop_size_h',
            'padding', 'top_padding_size', 'bottom_padding_size', 'left_padding_size', 'right_padding_size',
            'var_reci_chn_0', 'var_reci_chn_1', 'var_reci_chn_2', 'var_reci_chn_3'
        ]
        self.manager._get_int_safe = MagicMock(side_effect=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15])
        self.manager._get_float_safe = MagicMock(side_effect=[1.0, 2.0, 3.0, 4.0])

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_init(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 1
        manager = DymAippManager(session, config_file, batchsize)
        self.assertEqual(manager.cfg, mock_config_parser.return_value)
        self.assertEqual(manager.session, session)
        self.assertEqual(manager.batchsize, batchsize)

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_load_aipp_config_content(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 32

        # Mock ConfigParser
        mock_config_parser.return_value.sections.return_value = ['aipp_op']
        mock_config_parser.return_value.options.return_value = ['input_format', 'src_image_size_w', 'src_image_size_h']

        # Create DymAippManager instance
        manager = DymAippManager(session, config_file, batchsize)

        # Mock private methods
        manager._aipp_set_input_format = MagicMock()
        manager._aipp_set_src_image_size = MagicMock()
        manager._aipp_set_rbuv_swap_switch = MagicMock()
        manager._aipp_set_ax_swap_switch = MagicMock()
        manager._aipp_set_csc_params = MagicMock()
        manager._aipp_set_crop_params = MagicMock()
        manager._aipp_set_padding_params = MagicMock()
        manager._aipp_set_dtc_pixel_mean = MagicMock()
        manager._aipp_set_dtc_pixel_min = MagicMock()
        manager._aipp_set_pixel_var_reci = MagicMock()

        # Call method
        manager.load_aipp_config_content()

        # Assert
        manager._aipp_set_input_format.assert_called_once()

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_get_int_safe(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 1
        manager = DymAippManager(session, config_file, batchsize)

        # Mock config parser
        mock_config_parser.return_value.getint.return_value = 10

        # Test get_int_safe
        value = manager._get_int_safe('aipp_op', 'src_image_size_w')
        self.assertEqual(value, 10)

        # Test get_int_safe with exception
        mock_config_parser.return_value.getint.side_effect = Exception("Test exception")
        with self.assertRaises(ValueError):
            manager._get_int_safe('aipp_op', 'src_image_size_w')

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_get_float_safe(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 1
        manager = DymAippManager(session, config_file, batchsize)

        # Mock config parser
        mock_config_parser.return_value.getfloat.return_value = 10.5

        # Test get_float_safe
        value = manager._get_float_safe('aipp_op', 'min_chn_0')
        self.assertEqual(value, 10.5)

        # Test get_float_safe with exception
        mock_config_parser.return_value.getfloat.side_effect = Exception("Test exception")
        with self.assertRaises(ValueError):
            manager._get_float_safe('aipp_op', 'min_chn_0')

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_input_format(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 1
        manager = DymAippManager(session, config_file, batchsize)

        # Mock config parser
        mock_config_parser.return_value.get.return_value = "YUV420SP_U8"

        # Test aipp_set_input_format
        manager._aipp_set_input_format()
        session.aipp_set_input_format.assert_called_once_with("YUV420SP_U8")

        # Test invalid input format
        mock_config_parser.return_value.get.return_value = "INVALID_FORMAT"
        with self.assertRaises(ValueError):
            manager._aipp_set_input_format()

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_src_image_size(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 1
        manager = DymAippManager(session, config_file, batchsize)

        # Mock config parser
        mock_config_parser.return_value.getint.side_effect = [1024, 768]

        # Test aipp_set_src_image_size
        manager._aipp_set_src_image_size()
        session.aipp_set_src_image_size.assert_called_once_with([1024, 768])

        # Test invalid image size
        mock_config_parser.return_value.getint.side_effect = [4097, 768]
        with self.assertRaises(ValueError):
            manager._aipp_set_src_image_size()

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_rbuv_swap_switch(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 1
        manager = DymAippManager(session, config_file, batchsize)

        # Mock config parser
        mock_config_parser.return_value.options.return_value = ['rbuv_swap_switch']
        mock_config_parser.return_value.getint.return_value = 1

        # Test aipp_set_rbuv_swap_switch
        manager._aipp_set_rbuv_swap_switch(['rbuv_swap_switch'])
        session.aipp_set_rbuv_swap_switch.assert_called_once_with(1)

        # Test invalid switch value
        mock_config_parser.return_value.getint.return_value = 2
        with self.assertRaises(ValueError):
            manager._aipp_set_rbuv_swap_switch(['rbuv_swap_switch'])

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_ax_swap_switch(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 1
        manager = DymAippManager(session, config_file, batchsize)

        # Mock config parser
        mock_config_parser.return_value.options.return_value = ['ax_swap_switch']
        mock_config_parser.return_value.getint.return_value = 1

        # Test aipp_set_ax_swap_switch
        manager._aipp_set_ax_swap_switch(['ax_swap_switch'])
        session.aipp_set_ax_swap_switch.assert_called_once_with(1)

        # Test invalid switch value
        mock_config_parser.return_value.getint.return_value = 2
        with self.assertRaises(ValueError):
            manager._aipp_set_ax_swap_switch(['ax_swap_switch'])

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_csc_params(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 32

        # Mock ConfigParser
        mock_config_parser.return_value.sections.return_value = ['aipp_op']
        mock_config_parser.return_value.options.return_value = ['input_format', 'src_image_size_w', 'src_image_size_h']

        # Create DymAippManager instance
        manager = DymAippManager(session, config_file, batchsize)

        # Mock private method
        manager._get_int_safe = MagicMock(side_effect=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15])

        # Call method
        manager._aipp_set_csc_params(mock_config_parser.return_value.options.return_value)

        # Assert
        session.aipp_set_csc_params.assert_called_once()

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_crop_params(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 32

        # Mock ConfigParser
        mock_config_parser.return_value.sections.return_value = ['aipp_op']
        mock_config_parser.return_value.options.return_value = ['input_format', 'src_image_size_w', 'src_image_size_h']

        # Create DymAippManager instance
        manager = DymAippManager(session, config_file, batchsize)

        # Mock private method
        manager._get_int_safe = MagicMock(side_effect=[1, 2, 3, 4, 5])

        # Call method
        manager._aipp_set_crop_params(mock_config_parser.return_value.options.return_value)

        # Assert
        session.aipp_set_crop_params.assert_called_once()

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_padding_params(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 32

        # Mock ConfigParser
        mock_config_parser.return_value.sections.return_value = ['aipp_op']
        mock_config_parser.return_value.options.return_value = ['input_format', 'src_image_size_w', 'src_image_size_h']

        # Create DymAippManager instance
        manager = DymAippManager(session, config_file, batchsize)

        # Mock private method
        manager._get_int_safe = MagicMock(side_effect=[1, 2, 3, 4, 5])

        # Call method
        manager._aipp_set_padding_params(mock_config_parser.return_value.options.return_value)

        # Assert
        session.aipp_set_padding_params.assert_called_once()

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_dtc_pixel_mean(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 32

        # Mock ConfigParser
        mock_config_parser.return_value.sections.return_value = ['aipp_op']
        mock_config_parser.return_value.options.return_value = ['input_format', 'src_image_size_w', 'src_image_size_h']

        # Create DymAippManager instance
        manager = DymAippManager(session, config_file, batchsize)

        # Mock private method
        manager._get_int_safe = MagicMock(side_effect=[1, 2, 3, 4])

        # Call method
        manager._aipp_set_dtc_pixel_mean(mock_config_parser.return_value.options.return_value)

        # Assert
        session.aipp_set_dtc_pixel_mean.assert_called_once()

    @patch('ais_bench.infer.dym_aipp_manager.ConfigParser')
    def test_aipp_set_dtc_pixel_min(self, mock_config_parser):
        # Mock session and config file
        session = MagicMock()
        config_file = "path/to/config.ini"
        batchsize = 32

        # Mock ConfigParser
        mock_config_parser.return_value.sections.return_value = ['aipp_op']
        mock_config_parser.return_value.options.return_value = ['input_format', 'src_image_size_w', 'src_image_size_h']

        # Create DymAippManager instance
        manager = DymAippManager(session, config_file, batchsize)

        # Mock private method
        manager._get_float_safe = MagicMock(side_effect=[1.0, 2.0, 3.0, 4.0])

        # Call method
        manager._aipp_set_dtc_pixel_min(mock_config_parser.return_value.options.return_value)

        # Assert
        session.aipp_set_dtc_pixel_min.assert_called_once()

    def test_aipp_set_crop_params1(self):
        option_list = self.manager.cfg.options.return_value
        self.manager._aipp_set_crop_params(option_list)

        # Assert
        self.session.aipp_set_crop_params.assert_called_once_with([1, 2, 3, 4, 5])

    def test_aipp_set_padding_params1(self):
        option_list = self.manager.cfg.options.return_value
        self.manager._aipp_set_padding_params(option_list)

        # Assert
        self.session.aipp_set_padding_params.assert_called_once_with([1, 2, 3, 4, 5])

    def test_aipp_set_pixel_var_reci1(self):
        option_list = self.manager.cfg.options.return_value
        self.manager._aipp_set_pixel_var_reci(option_list)

        # Assert
        self.session.aipp_set_pixel_var_reci.assert_called_once_with([1.0, 2.0, 3.0, 4.0])

    @patch('ais_bench.infer.dym_aipp_manager.logger.error')
    def test_aipp_set_csc_params_invalid_switch1(self, mock_logger_error):
        option_list = self.manager.cfg.options.return_value
        self.manager._get_int_safe.side_effect = [2]  # Invalid csc_switch value

        with self.assertRaises(ValueError):
            self.manager._aipp_set_csc_params(option_list)

        mock_logger_error.assert_called_once_with("csc_switch in config file out of range, please check it!")

    @patch('ais_bench.infer.dym_aipp_manager.logger.error')
    def test_aipp_set_crop_params_invalid_switch1(self, mock_logger_error):
        option_list = self.manager.cfg.options.return_value
        self.manager._get_int_safe.side_effect = [2]  # Invalid crop value

        with self.assertRaises(ValueError):
            self.manager._aipp_set_crop_params(option_list)

        mock_logger_error.assert_called_once_with("crop_switch(crop) in config file out of range, please check it!")

    @patch('ais_bench.infer.dym_aipp_manager.logger.error')
    def test_aipp_set_padding_params_invalid_switch1(self, mock_logger_error):
        option_list = self.manager.cfg.options.return_value
        self.manager._get_int_safe.side_effect = [2]  # Invalid padding value

        with self.assertRaises(ValueError):
            self.manager._aipp_set_padding_params(option_list)

        mock_logger_error.assert_called_once_with("padding_switch in config file out of range, please check it!")
