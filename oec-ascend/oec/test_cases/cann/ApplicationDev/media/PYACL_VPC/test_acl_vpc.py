# -*- coding:utf-8 -*-
import os
import unittest
import numpy as np
from decimal import Decimal, getcontext
import acl
import utils as util
from utils import align_size
from utils import get_align_size
import sys

data_path = sys.argv[1]
print(f"data path is {data_path}")

YUV400 = 0
YUV420 = 1
YUV422 = 3
ACL_MEMCPY_HOST_TO_DEVICE = 1
ACL_MEMCPY_DEVICE_TO_HOST = 2
HEIGHT_STRIDE = (1, 2)
ONE_PIXEL_OCCUPY_TWO_BYTE = 2
ONE_PIXEL_OCCUPY_THREE_BYTE = 3
ONE_PIXEL_OCCUPY_FOUR_BYTE = 4

WIDTH_STRIDE = {
    (0, 1, 2, 3, 4, 5, 6, 1000, 1001):lambda x,y: align_size(y, 16),
    (7, 8, 9, 10): lambda x,y: align_size(y, 16) * ONE_PIXEL_OCCUPY_TWO_BYTE,
    (11, 12, 13):lambda x,y: align_size(y, 16) * ONE_PIXEL_OCCUPY_THREE_BYTE,
    (14, 15, 16, 17):lambda x,y: align_size(y, 16) * ONE_PIXEL_OCCUPY_FOUR_BYTE
}

BUFFER_SIZE = {
    (0, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17):lambda x, y:x * y,
    (1, 2):lambda x, y:(x * y) * 3 // 2,
    (3, 4):lambda x, y:(x * y) * 2,
    (6):lambda x, y:(x * y) * 3,
}


class AclVpc(object):
    def __init__(self, in_batch_size=1, out_batch_size=1):
        self.out_batch_size = out_batch_size
        self.in_batch_size = in_batch_size
        self.out_batch_pic_desc = None
        self.in_batch_pic_desc = None
        self.crop_area = None
        self.paste_area = None
        self.output_desc = None
        self.input_desc = None
        self.in_buffer_dev = None
        self.out_buffer_dev = None
        self.dvpp_channel_desc = None
        self.resize_config = None
        self.context, ret = acl.rt.create_context(0)
        assert ret == 0
        acl.rt.set_context(self.context)
        self.stream, ret = acl.rt.create_stream()
        assert ret == 0
        self.dev_buffer = {}
        self.corpList, self.pasteList = [], []

    def __del__(self):
        acl.rt.set_context(self.context)
        self._free_pic_desc()
        for i in range(len(self.corpList)):
            ret = acl.media.dvpp_destroy_roi_config(self.corpList[i])
            assert ret == 0
        for i in range(len(self.pasteList)):
            ret = acl.media.dvpp_destroy_roi_config(self.pasteList[i])
            assert ret == 0
        for key in self.dev_buffer.keys():
            if self.dev_buffer[key]:
                ret = acl.media.dvpp_free(self.dev_buffer[key])
                assert ret == 0
        if self.resize_config:
            ret = acl.media.dvpp_destroy_resize_config(self.resize_config)
            assert ret == 0
        roi_conf = [self.crop_area, self.paste_area]
        for i in range(len(roi_conf)):
            if roi_conf[i]:
                ret = acl.media.dvpp_destroy_roi_config(roi_conf[i])
                assert ret == 0
        buffer_dev = [self.in_buffer_dev, self.out_buffer_dev]
        for i in range(len(buffer_dev)):
            if buffer_dev[i]:
                ret = acl.media.dvpp_free(buffer_dev[i])
                assert ret == 0
        if self.dvpp_channel_desc:
            ret = acl.media.dvpp_destroy_channel(self.dvpp_channel_desc)
            assert ret == 0
            ret = acl.media.dvpp_destroy_channel_desc(self.dvpp_channel_desc)
            assert ret == 0
        ret = acl.rt.destroy_stream(self.stream)
        assert ret == 0
        ret = acl.rt.destroy_context(self.context)
        assert ret == 0
        print("vpc free resource")

    def _free_pic_desc(self):
        desc = [self.output_desc, self.input_desc]
        for i in range(len(desc)):
            if desc[i]:
                ret = acl.mdeia.dvpp_destroy_pic_desc(desc[i])
                assert ret == 0
        
        batch_pic_desc = [self.out_batch_pic_desc, self.in_batch_pic_desc]
        for i in range(len(batch_pic_desc)):
            if batch_pic_desc[i]:
                ret = acl.media.dvpp_destroy_batch_pic_desc(batch_pic_desc[i])
                assert ret == 0

    def dvpp_set_pic_desc(self, desc, buffer, width, height, wstride, hstride, size, format=YUV420):
        ret = acl.media.dvpp_set_pic_desc_data(desc, buffer)
        assert ret == 0
        ret = acl.media.dvpp_set_pic_desc_format(desc, format)
        assert ret == 0
        ret = acl.media.dvpp_set_pic_desc_width(desc, width)
        assert ret == 0
        ret = acl.media.dvpp_set_pic_desc_height(desc, height)
        assert ret == 0
        ret = acl.media.dvpp_set_pic_desc_width_stride(desc, wstride)
        assert ret == 0
        ret = acl.media.dvpp_set_pic_desc_height_stride(desc, hstride)
        assert ret == 0
        ret = acl.media.dvpp_set_pic_desc_size(desc, size)
        assert ret == 0

    def vpc_init(self):
        acl.rt.set_context(self.context)

        #create channel desc
        self.dvpp_channel_desc = acl.media.dvpp_create_channel_desc()
        assert self.dvpp_channel_desc != 0

        #create channel desc
        ret = acl.media.dvpp_create_channel(self.dvpp_channel_desc)
        assert ret == 0
    
    def get_picture_height_stride(self, format, height):
        """
        get picture height stride
            1.YUV420 height stride 2 alignment
            2.other format height stride no aligment.
        """
        if format in HEIGHT_STRIDE:
            return int(((height + 1) // 2) * 2)
        return int(height)
    
    def get_picture_width_stride(self, format, width):
        """
        get picture width stride:
        1.width stride 16 alignment,
        2.width stride 16 alignment, one PIXEL occupy two byte,
        3.width stride 16 alignment, one PIXEL occupy three byte,
        4.width stride 16 alignment, one PIXEL occupy four byte,
        """
        return get_align_size(WIDTH_STRIDE, format, 0, width)

    def get_picture_buffer_size(self, format, width_stride, height_stride, flag):
        """
        get pictutre buffer size:
            1.YUV400 in 310P memory is width_stride * height_stride
            2.YUV400,YUV420 memory is width_stride * height_stride * 3 //2
            3.YUV422SP,YUV440SP  memory is width_stride * height_stride * 2
            4.YUV4442SP  memory is width_stride * height_stride * 3
            5.other support format memory is width_stride * height_stride.
        """
        if flag:
            return width_stride * height_stride * 3 // 2
        return get_align_size(BUFFER_SIZE,
                                format,
                                width_stride,
                                height_stride)

    def set_picture_desc(self, desc, width, height, opt, i, format=YUV420, flag=True):
        """"get picture info and set picture description"""
        width_stride = self.get_picture_width_stride(format, width)
        height_stride = self.get_picture_height_stride(format, height)
        buffer_size = self.get_picture_buffer_size(format,
                                                    width_stride,
                                                    height_stride,
                                                    flag)
        buffer_size = int(buffer_size)
        dev, ret =acl.media.dvpp_malloc(buffer_size)
        assert ret == 0
        ret = acl.rt.memset(dev, buffer_size, 0, buffer_size)
        assert ret == 0
        key = opt + '_' + str(i)
        self.dev_buffer[key] = dev
        self.dvpp_set_pic_desc(desc, dev, width,
                                height, width_stride, height_stride,
                                buffer_size, format)
        return buffer_size      

    def get_pic_desc_data(self, pic_desc):
        pic_data = acl.media.dvpp_get_pic_desc_data(pic_desc)
        pic_data_size = acl.media.dvpp_get_pic_desc_size(pic_desc)
        ret_code = acl.media.dvpp_get_pic_desc_ret_code(pic_desc)
        assert ret_code == 0

        # pic memcpy d2h
        np_pic = np.zeros(pic_data_size, dtype=np.byte)
        bytes_data = np_pic.tobytes()
        np_pic_ptr = acl.util.bytes_to_ptr(bytes_data)
        ret = acl.rt.memcpy(np_pic_ptr, pic_data_size,
                                pic_data, pic_data_size, ACL_MEMCPY_DEVICE_TO_HOST)
        assert ret == 0
        return np_pic
        
    def async_vpc_batch_crop_resize_paste_synchronize(self, w, h, path):
        self.out_batch_pic_desc = acl.media.dvpp_create_batch_pic_desc(self.out_batch_size)
        self.in_batch_pic_desc = acl.media.dvpp_create_batch_pic_desc(self.in_batch_size)
        # load data from file
        np_yuv = np.fromfile(path, dtype=np.byte)
        in_buffer_size = np_yuv.itemsize * np_yuv.size
        bytes_data = np_yuv.tobytes()
        bytes_yuv_ptr = acl.util.bytes_to_ptr(bytes_data)
        roiList = []
        for i in range(self.in_batch_size):
            input_desc = acl.media.dvpp_get_pic_desc(self.in_batch_pic_desc, i)
            print(self.in_batch_pic_desc, input_desc, i)
            assert input_desc != 0
            self.set_picture_desc(input_desc, w, h, "input", i)
            #copy from host to device
            key = "input" + '_' + str(i)
            ret = acl.rt.memcpy(self.dev_buffer[key], in_buffer_size, bytes_yuv_ptr,
                                in_buffer_size, ACL_MEMCPY_HOST_TO_DEVICE)
            assert ret == 0
            roiList.append(self.out_batch_size // self.in_batch_size)
        
        for i in range(self.out_batch_size):
            out_desc = acl.media.dvpp_get_pic_desc(self.out_batch_pic_desc, i)
            assert out_desc != 0
            self.set_picture_desc(out_desc, w // 2, h // 2, "output", i)
            if i % 2 == 0:
                crop_area = acl.media.dvpp_create_roi_config(w // 2, w - 1, h // 2, h - 1)
                paste_area = acl.media.dvpp_create_roi_config(w // 4, w // 2 - 1,
                                                              h // 4, h // 2 - 1 )
            else:
                crop_area = acl.media.dvpp_create_roi_config(0, w // 2 -1, 0, h // 2 -1)
                paste_area = acl.media.dvpp_create_roi_config(0, w // 4 - 1, 0, h // 4 -1)
            self.corpList.append(crop_area)
            self.pasteList.append(paste_area)

        total_num = 0
        for i in range(self.in_batch_size):
            total_num += roiList[i]
        if self.out_batch_size % self.in_batch_size != 0:
            roiList[-1] = self.out_batch_size - total_num + roiList[-1]

        self.resize_config = acl.media.dvpp_create_resize_config()
        ret = acl.media.dvpp_vpc_batch_crop_resize_paste_async(self.dvpp_channel_desc, self.in_batch_pic_desc,
                                                               roiList, self.out_batch_pic_desc, self.corpList,
                                                               self.pasteList, self.resize_config, self.stream)
        print("ret:",ret)
        assert ret == 0
        ret = acl.rt.synchronize_stream(self.stream)
        assert ret == 0
        np_list = []
        for i in range(self.out_batch_size):
            output_desc = acl.media.dvpp_get_pic_desc(self.out_batch_pic_desc, i)
            np_output = self.get_pic_desc_data(output_desc)
            np_list.append(np_output)

        return np_list


class TestVpc(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """called only once before all testcase"""
        # init
        ret = acl.init("")
        assert ret == 0
        ret = acl.rt.set_device(0)
        assert ret == 0

    @classmethod
    def tearDownClass(cls):
        """ called only once after all testcase """
        ret = acl.rt.reset_device(0)
        assert ret == 0
        ret = acl.finalize()
        assert ret == 0

    def setUp(self) -> None:
        pass
    
    def tearDown(self) -> None:
        pass


    def test_vpc_019_batch_crop_resize_paste_1_batch_input(self):
        """
        test case for vpc batch crop resize paste
        """
        vpc_handle = AclVpc(1, 2)
        vpc_handle.vpc_init()
        # 512x368 -> 256x184(crop) -> 128x92(resize) -> 256x184(paste)
        out = vpc_handle.async_vpc_batch_crop_resize_paste_synchronize(1024, 368, f"{data_path}/data/wood_rabbit_1024_1068_nv12.yuv")
        print("out:", out)
        device_type = util.get_device_type()



if __name__ == "__main__":
    #util.show_growth()
    suite = util.switch_cases(TestVpc, "all")
    unittest.TextTestRunner(verbosity=2).run(suite)
    #util.show_growth()