# Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from ais_bench.infer.common.logger import logger
from configparser import ConfigParser
import aclruntime

SRC_IMAGE_SIZE_W_MIN = 2
SRC_IMAGE_SIZE_W_MAX = 4096
SRC_IMAGE_SIZE_H_MIN = 1
SRC_IMAGE_SIZE_H_MAX = 4096
RBUV_SWAP_SWITCH_OFF = 0
RBUV_SWAP_SWITCH_ON = 1
AX_SWAP_SWITCH_OFF = 0
AX_SWAP_SWITCH_ON = 1
CSC_SWITCH_OFF = 0
CSC_SWITCH_ON = 1
CSC_MATRIX_MIN = -32677
CSC_MATRIX_MAX = 32676
CROP_SWITCH_OFF = 0
CROP_SWITCH_ON = 1
LOAD_START_POS_W_MIN = 0
LOAD_START_POS_W_MAX = 4095
LOAD_START_POS_H_MIN = 0
LOAD_START_POS_H_MAX = 4095
CROP_POS_W_MIN = 1
CROP_POS_W_MAX = 4096
CROP_POS_H_MIN = 1
CROP_POS_H_MAX = 4096
PADDING_SWITCH_OFF = 0
PADDING_SWITCH_ON = 1
PADDING_SIZE_MIN = 0
PADDING_SIZE_MAX = 32
PIXEL_MEAN_CHN_MIN = 0
PIXEL_MEAN_CHN_MAX = 255
PIXEL_MIN_CHN_MIN = 0
PIXEL_MIN_CHN_MAX = 255
PIXEL_VAR_RECI_CHN_MIN = -65504
PIXEL_VAR_RECI_CHN_MAX = 65504


class DymAippManager:
    def __init__(self, session, config_file, batchsize):
        self.cfg = ConfigParser()
        self.cfg.read(config_file, 'UTF-8')
        self.session = session
        self.batchsize = batchsize

    def load_aipp_config_content(self):
        session_list = self.cfg.sections()
        #多个aipp输入不支持
        if (session_list.count('aipp_op') != 1):
            logger.error("nums of section aipp_op in .config file is not supported, please check it!")
            raise RuntimeError('wrong aipp config file content!')
        option_list = self.cfg.options('aipp_op')
        if (option_list.count('input_format') == 1):
            self._aipp_set_input_format()
        else:
            logger.error("can not find input_format in config file, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        if (option_list.count('src_image_size_w') == 1 and option_list.count('src_image_size_h') == 1):
            self._aipp_set_src_image_size()
        else:
            logger.error("can not find src_image_size in config file, please check it!")
            raise RuntimeError('wrong aipp config file content!')
        self.session.aipp_set_max_batch_size(self.batchsize)
        self._aipp_set_rbuv_swap_switch(option_list)
        self._aipp_set_ax_swap_switch(option_list)
        self._aipp_set_csc_params(option_list)
        self._aipp_set_crop_params(option_list)
        self._aipp_set_padding_params(option_list)
        self._aipp_set_dtc_pixel_mean(option_list)
        self._aipp_set_dtc_pixel_min(option_list)
        self._aipp_set_pixel_var_reci(option_list)

    def _aipp_set_input_format(self):
        input_format = self.cfg.get('aipp_op', 'input_format')
        legal_format = ["YUV420SP_U8", "XRGB8888_U8", "RGB888_U8", "YUV400_U8"]
        if (legal_format.count(input_format) == 1):
            self.session.aipp_set_input_format(input_format)
        else:
            logger.error("input_format in config file is illegal, please check it!")
            raise RuntimeError('wrong aipp config file content!')

    def _aipp_set_src_image_size(self):
        src_image_size = list()
        tmp_size_w = self.cfg.getint('aipp_op', 'src_image_size_w')
        tmp_size_h = self.cfg.getint('aipp_op', 'src_image_size_h')
        if (SRC_IMAGE_SIZE_W_MIN <= tmp_size_w <= SRC_IMAGE_SIZE_W_MAX):
            src_image_size.append(tmp_size_w)
        else:
            logger.error("src_image_size_w in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')
        if (SRC_IMAGE_SIZE_H_MIN <= tmp_size_h <= SRC_IMAGE_SIZE_H_MAX):
            src_image_size.append(tmp_size_h)
        else:
            logger.error("src_image_size_h in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        self.session.aipp_set_src_image_size(src_image_size)

    def _aipp_set_rbuv_swap_switch(self, option_list):
        if (option_list.count('rbuv_swap_switch') == 0):
            self.session.aipp_set_rbuv_swap_switch(RBUV_SWAP_SWITCH_OFF)
            return
        tmp_rs_switch = self.cfg.getint('aipp_op', 'rbuv_swap_switch')
        if (tmp_rs_switch == RBUV_SWAP_SWITCH_OFF or tmp_rs_switch == RBUV_SWAP_SWITCH_ON):
            self.session.aipp_set_rbuv_swap_switch(tmp_rs_switch)
        else:
            logger.error("rbuv_swap_switch in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

    def _aipp_set_ax_swap_switch(self, option_list):
        if (option_list.count('ax_swap_switch') == 0):
            self.session.aipp_set_ax_swap_switch(AX_SWAP_SWITCH_OFF)
            return
        tmp_as_switch = self.cfg.getint('aipp_op', 'ax_swap_switch')
        if (tmp_as_switch == AX_SWAP_SWITCH_OFF or tmp_as_switch == AX_SWAP_SWITCH_ON):
            self.session.aipp_set_ax_swap_switch(tmp_as_switch)
        else:
            logger.error("ax_swap_switch in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

    def _aipp_set_csc_params(self, option_list):
        if (option_list.count('csc_switch') == 0):
            tmp_csc_switch = CSC_SWITCH_OFF
        else:
            tmp_csc_switch = self.cfg.getint('aipp_op', 'csc_switch')

        if (tmp_csc_switch == CSC_SWITCH_OFF):
            tmp_csc_params = [0] * 16
        elif (tmp_csc_switch == CSC_SWITCH_ON):
            tmp_csc_params = list()
            tmp_csc_params.append(tmp_csc_switch)
            options = [
                'matrix_r0c0', 'matrix_r0c1', 'matrix_r0c2', 'matrix_r1c0', 'matrix_r1c1', 'matrix_r1c2',
                'matrix_r2c0', 'matrix_r2c1', 'matrix_r2c2', 'output_bias_0', 'output_bias_1', 'output_bias_2',
                'input_bias_0', 'input_bias_1', 'input_bias_2'
            ]
            for option in options:
                tmp_csc_params.append(0 if option_list.count(option) == 0 else self.cfg.getint('aipp_op', option))

            range_ok = True
            for i in range(1, 9):
                range_ok = range_ok and (CSC_MATRIX_MIN <= tmp_csc_params[i] <= CSC_MATRIX_MAX)
            for i in range(10, 15):
                range_ok = range_ok and (0 <= tmp_csc_params[i] <= 255)
            if (range_ok is False):
                logger.error("csc_params in config file out of range, please check it!")
                raise RuntimeError('wrong aipp config file content!')
        else:
            logger.error("csc_switch in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        self.session.aipp_set_csc_params(tmp_csc_params)

    def _aipp_set_crop_params(self, option_list):
        if (option_list.count('crop') == 0):
            tmp_crop_switch = CROP_SWITCH_OFF
        else:
            tmp_crop_switch = self.cfg.getint('aipp_op', 'crop')

        if (tmp_crop_switch == CROP_SWITCH_OFF):
            tmp_crop_params = [0, 0, 0, 416, 416]
        elif (tmp_crop_switch == CROP_SWITCH_ON):
            tmp_crop_params = list()
            tmp_crop_params.append(tmp_crop_switch)
            tmp_crop_params.append(
                0 if option_list.count('load_start_pos_w') == 0 else self.cfg.getint('aipp_op', 'load_start_pos_w')
            )
            tmp_crop_params.append(
                0 if option_list.count('load_start_pos_h') == 0 else self.cfg.getint('aipp_op', 'load_start_pos_h')
            )
            tmp_crop_params.append(
                0 if option_list.count('crop_size_w') == 0 else self.cfg.getint('aipp_op', 'crop_size_w')
            )
            tmp_crop_params.append(
                0 if option_list.count('crop_size_h') == 0 else self.cfg.getint('aipp_op', 'crop_size_h')
            )

            range_ok = True
            range_ok = range_ok and (LOAD_START_POS_W_MIN <= tmp_crop_params[1] <= LOAD_START_POS_W_MAX)
            range_ok = range_ok and (LOAD_START_POS_H_MIN <= tmp_crop_params[2] <= LOAD_START_POS_H_MAX)
            range_ok = range_ok and (CROP_POS_W_MIN <= tmp_crop_params[3] <= CROP_POS_W_MAX)
            range_ok = range_ok and (CROP_POS_H_MIN <= tmp_crop_params[4] <= CROP_POS_H_MAX)
            if (range_ok is False):
                logger.error("crop_params in config file out of range, please check it!")
                raise RuntimeError('wrong aipp config file content!')
        else:
            logger.error("crop_switch(crop) in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        self.session.aipp_set_crop_params(tmp_crop_params)

    def _aipp_set_padding_params(self, option_list):
        if (option_list.count('padding') == 0):
            tmp_padding_switch = PADDING_SWITCH_OFF
        else:
            tmp_padding_switch = self.cfg.getint('aipp_op', 'padding')

        if (tmp_padding_switch == PADDING_SWITCH_OFF):
            tmp_padding_params = [0] * 5
        elif (tmp_padding_switch == PADDING_SWITCH_ON):
            tmp_padding_params = list()
            tmp_padding_params.append(tmp_padding_switch)
            tmp_padding_params.append(
                0 if option_list.count('top_padding_size') == 0 else self.cfg.getint('aipp_op', 'top_padding_size')
            )
            tmp_padding_params.append(
                0 if option_list.count('bottom_padding_size') == 0 else self.cfg.getint('aipp_op', 'bottom_padding_size')
            )
            tmp_padding_params.append(
                0 if option_list.count('left_padding_size') == 0 else self.cfg.getint('aipp_op', 'left_padding_size')
            )
            tmp_padding_params.append(
                0 if option_list.count('right_padding_size') == 0 else self.cfg.getint('aipp_op', 'right_padding_size')
            )

            range_ok = True
            for i in range(1, 5):
                range_ok = range_ok and (PADDING_SIZE_MIN <= tmp_padding_params[i] <= PADDING_SIZE_MAX)
            if (range_ok is False):
                logger.error("padding_params in config file out of range, please check it!")
                raise RuntimeError('wrong aipp config file content!')
        else:
            logger.error("padding_switch in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        self.session.aipp_set_padding_params(tmp_padding_params)

    def _aipp_set_dtc_pixel_mean(self, option_list):
        tmp_mean_params = list()
        tmp_mean_params.append(
            0 if option_list.count('mean_chn_0') == 0 else self.cfg.getint('aipp_op', 'mean_chn_0')
        )
        tmp_mean_params.append(
            0 if option_list.count('mean_chn_1') == 0 else self.cfg.getint('aipp_op', 'mean_chn_1')
        )
        tmp_mean_params.append(
            0 if option_list.count('mean_chn_2') == 0 else self.cfg.getint('aipp_op', 'mean_chn_2')
        )
        tmp_mean_params.append(
            0 if option_list.count('mean_chn_3') == 0 else self.cfg.getint('aipp_op', 'mean_chn_3')
        )

        range_ok = True
        for i in range(0, 4):
            range_ok = range_ok and (PIXEL_MEAN_CHN_MIN <= tmp_mean_params[i] <= PIXEL_MEAN_CHN_MAX)
        if (range_ok is False):
            logger.error("mean_chn_params in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        self.session.aipp_set_dtc_pixel_mean(tmp_mean_params)

    def _aipp_set_dtc_pixel_min(self, option_list):
        tmp_min_params = list()
        tmp_min_params.append(
            0 if option_list.count('min_chn_0') == 0 else self.cfg.getfloat('aipp_op', 'min_chn_0')
        )
        tmp_min_params.append(
            0 if option_list.count('min_chn_1') == 0 else self.cfg.getfloat('aipp_op', 'min_chn_1')
        )
        tmp_min_params.append(
            0 if option_list.count('min_chn_2') == 0 else self.cfg.getfloat('aipp_op', 'min_chn_2')
        )
        tmp_min_params.append(
            0 if option_list.count('min_chn_3') == 0 else self.cfg.getfloat('aipp_op', 'min_chn_3')
        )

        range_ok = True
        for i in range(0, 4):
            range_ok = range_ok and (PIXEL_MIN_CHN_MIN <= tmp_min_params[i] <= PIXEL_MIN_CHN_MAX)
        if (range_ok is False):
            logger.error("min_chn_params in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        self.session.aipp_set_dtc_pixel_min(tmp_min_params)

    def _aipp_set_pixel_var_reci(self, option_list):
        tmp_reci_params = list()
        tmp_reci_params.append(
            0 if option_list.count('var_reci_chn_0') == 0 else self.cfg.getfloat('aipp_op', 'var_reci_chn_0')
        )
        tmp_reci_params.append(
            0 if option_list.count('var_reci_chn_1') == 0 else self.cfg.getfloat('aipp_op', 'var_reci_chn_1')
        )
        tmp_reci_params.append(
            0 if option_list.count('var_reci_chn_2') == 0 else self.cfg.getfloat('aipp_op', 'var_reci_chn_2')
        )
        tmp_reci_params.append(
            0 if option_list.count('var_reci_chn_3') == 0 else self.cfg.getfloat('aipp_op', 'var_reci_chn_3')
        )

        range_ok = True
        for i in range(0, 4):
            range_ok = range_ok and (PIXEL_VAR_RECI_CHN_MIN <= tmp_reci_params[i] <= PIXEL_VAR_RECI_CHN_MAX)
        if (range_ok is False):
            logger.error("var_reci_chn_params in config file out of range, please check it!")
            raise RuntimeError('wrong aipp config file content!')

        self.session.aipp_set_pixel_var_reci(tmp_reci_params)

