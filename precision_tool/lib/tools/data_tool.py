import numpy as np
import logging
import struct
from ..util.util import util
import torch

opt= torch.optim.AdamW()


class DataTool(object):
    def __init__(self):
        self.log = logging.getLogger()

    def fp2bin(self, data, ):
        binary_representation = format(struct.unpack('>I', struct.pack('>f', float_value))[0], '032b')
        util.print_panel()


# 示例
float_value = 3.14
binary_representation = float_to_binary(float_value)
print(f"float值 {float_value} 的二进制表示为: {binary_representation}")
