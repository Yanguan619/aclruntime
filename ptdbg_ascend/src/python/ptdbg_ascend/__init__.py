#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
# Copyright (C) 2019-2020. Huawei Technologies Co., Ltd. All rights reserved.
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
# ==============================================================================
"""

import torch
from .compare.acc_compare import compare, parse
from .compare.distributed_compare import compare_distributed
from .dump.dump import acc_cmp_dump
from .overflow_check.overflow_check import overflow_check
from .overflow_check.utils import set_overflow_check_switch
from .dump.utils import set_dump_path, set_dump_switch, set_backward_input
from .hook_module.register_hook import register_hook
from .common.utils import seed_all
from .common.version import __version__
seed_all()

__all__ = ["register_hook", "set_dump_path", "set_dump_switch", "set_overflow_check_switch", "seed_all",
           "acc_cmp_dump", "overflow_check", "compare", "parse", "compare_distributed", "set_backward_input"]

if torch.__version__ > "1.8":
    import torch.nn.functional as F
    from torch import _VF
    from torch.overrides import has_torch_function_unary, handle_torch_function


    def dropout_forward(self, input):
        return function_dropout(input, 0., self.training, self.inplace)


    def dropout2d_forward(self, input):
        return function_dropout2d(input, 0., self.training, self.inplace)


    def dropout3d_forward(self, input):
        return function_dropout3d(input, 0., self.training, self.inplace)


    torch.nn.Dropout.forward = dropout_forward
    torch.nn.Dropout2d.forward = dropout2d_forward
    torch.nn.Dropout3d.forward = dropout3d_forward


    def function_dropout(input: torch.Tensor, p: float = 0.5, training: bool = True,
                         inplace: bool = False) -> torch.Tensor:
        if has_torch_function_unary(input):
            return handle_torch_function(function_dropout, (input,), input, p=0., training=training, inplace=inplace)
        if p < 0.0 or p > 1.0:
            raise ValueError("dropout probability has to be between 0 and 1, " "but got {}".format(p))
        return _VF.dropout_(input, 0., training) if inplace else _VF.dropout(input, 0., training)


    def function_dropout2d(input: torch.Tensor, p: float = 0.5, training: bool = True,
                           inplace: bool = False) -> torch.Tensor:
        if has_torch_function_unary(input):
            return handle_torch_function(function_dropout2d, (input,), input, p=0., training=training, inplace=inplace)
        if p < 0.0 or p > 1.0:
            raise ValueError("dropout probability has to be between 0 and 1, " "but got {}".format(p))
        return _VF.feature_dropout_(input, 0., training) if inplace else _VF.feature_dropout(input, 0., training)


    def function_dropout3d(input: torch.Tensor, p: float = 0.5, training: bool = True,
                           inplace: bool = False) -> torch.Tensor:
        if has_torch_function_unary(input):
            return handle_torch_function(function_dropout3d, (input,), input, p=0., training=training, inplace=inplace)
        if p < 0.0 or p > 1.0:
            raise ValueError("dropout probability has to be between 0 and 1, " "but got {}".format(p))
        return _VF.feature_dropout_(input, 0., training) if inplace else _VF.feature_dropout(input, 0., training)


    F.dropout = function_dropout
    F.dropout2d = function_dropout2d
    F.dropout3d = function_dropout3d
