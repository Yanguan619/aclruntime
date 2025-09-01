# -*- coding:utf-8 -*-
# 版权所有 (c) 华为技术有限公司 2022-2023
import unittest
from threading import Lock
import os
import shutil
import numpy as np
import acl
import utils as util
import sys
import subprocess
import json
#from constant import Const

data_path = sys.argv[1]
print(f"data path is {data_path}")

output_dir = sys.argv[2]
print(f"output dir is {output_dir}")

#get soc version
soc_version = acl.get_soc_name()
print(f"soc version is {soc_version}")

add_json="""
[
  {
    "op": "Add",
    "input_desc": [
      {
        "format": "ND",
        "shape": [8, 16],
        "type": "int32"
      },
      {
        "format": "ND",
        "shape": [8, 16],
        "type": "int32"
      }
    ],
    "output_desc": [
      {
        "format": "ND",
        "shape": [8, 16],
        "type": "int32"
      }
    ]
  }
]
"""
add_json = json.loads(add_json)

with open(f"{output_dir}/add.json", 'w', encoding='utf-8') as json_file:
    json.dump(add_json, json_file, ensure_ascii=False, indent=4)

#transfer op model
subprocess.run(f"atc --singleop={output_dir}/add.json --output={output_dir} --soc_version={soc_version}", shell=True, cwd=f"{output_dir}")


acl_dtype = {
    "float32": 0,
    "float16": 1,
    "int8": 2,
    "int32": 3,
    "uint8": 4,
    "int16": 6,
    "uint16": 7,
    "uint32": 8,
    "int64": 9,
    "double": 11,
    "bool": 12
}

ACL_FORMAT_UNDEFINED = -1
ACL_FORMAT_NCHW = 0
ACL_FORMAT_NHWC = 1
ACL_FORMAT_ND = 2
ACL_FORMAT_NC1HWC0 = 3
ACL_FORMAT_FRACTAL_Z = 4
ACL_MEMCPY_HOST_TO_HOST = 0
ACL_MEMCPY_HOST_TO_DEVICE = 1
ACL_MEMCPY_DEVICE_TO_HOST = 2
NPY_BYTE = 1
ACL_FORMAT_ND = 2
ACL_MEM_MALLOC_HUGE_FIRST = 0
ACL_MEM_MALLOC_NORMAL_ONLY = 2
ACL_STEP_START = 0
ACL_STEP_END = 1
ACL_OP_DUMP_OP_AICORE_ARGS = 0x00000001

def op_select(in_num, in_desc, out_num, out_desc, op_attr, op_kernel_desc):
    """
    operator selector
    """
    # get input
    tilling_args = []
    args_list = []
    for i in range(in_num):
        tilling_args.append(str(acl.get_tensor_desc_dim_v2(in_desc[i], 0)[0]))
        tilling_args.append(str(acl.get_tensor_desc_dim_v2(in_desc[i], 1)[0]))
        tilling_args.append(tilling_type[str(acl.get_tensor_desc_type(in_desc[i]))])
    
    # get output
    for i in range(out_num):
        tilling_args.append(str(acl.get_tensor_desc_dim_v2(out_desc[i], 0)[0]))
        tilling_args.append(str(acl.get_tensor_desc_dim_v2(out_desc[i], 1)[0]))
        tilling_args.append(tilling_type[str(acl.get_tensor_desc_type(out_desc[i]))])

    #set args
    tilling = '_'.join(tilling_args)
    args = np.array(args_list, dtype=np.uint32).tobytes()
    args_ptr = acl.util.bytes_to_ptr(args)
    size = len(args)
    ret = acl.op.set_kernel_args(op_kernel_desc, tilling_mode[tilling], 2, args_ptr, size)
    assert ret == 0
    workspace_sizes = bytes()
    workspace_sizes_ptr = acl.util.bytes_to_ptr(workspace_sizes)
    ret = acl.op.set_kernel_workspaces_sizes(op_kernel_desc, 0, workspace_sizes_ptr)
    assert ret == 0


class AclOp(object):
    def __init__(self, a, b):
        self.in_list = []
        self.in_host_list = []
        self.in_desc_list = []
        self.in_dev_list = []
        self.out_dev_list = []
        self.host_list = []
        self.out_list = []
        self.out_desc_list = []
        self.data = [a, b]
        self.type = a.dtype
        self.shape = a.shape
        self.spec_type = ACL_FORMAT_ND
        # attr
        self.attr = acl.op.create_attr()
        assert self.attr != 0
        # stream
        self.stream, ret = acl.rt.create_stream()
        assert ret == 0

    def __del__(self):
        # free resource
        for i in range(len(self.in_desc_list)):
            ret = acl.destroy_data_buffer(self.in_list[i])
            assert ret == 0
            ret = acl.destroy_data_buffer(self.in_host_list[i])
            assert ret == 0
            acl.destroy_tensor_desc(self.in_desc_list[i])
        
        for i in range(len(self.out_desc_list)):
            ret = acl.destroy_data_buffer(self.out_list[i])
            assert ret == 0
            acl.destroy_tensor_desc(self.out_desc_list[i])
        
        for i in range(len(self.in_dev_list)):
            ret = acl.rt.free(self.in_dev_list[i])
            assert ret == 0
        
        for i in range(len(self.out_dev_list)):
            ret = acl.rt.free(self.out_dev_list[i])
            assert ret == 0
        
        for i in range(len(self.host_list)):
            ret = acl.rt.free_host(self.host_list[i])
            assert ret == 0
        
        acl.op.destroy_attr(self.attr)

        ret = acl.rt.destroy_stream(self.stream)
        assert ret == 0

    def tensor_desc_init(self, gen_dataset=True):
        # create input output tensors
        for data in self.data:
            desc = acl.create_tensor_desc(acl_dtype[str(data.dtype)], list(data.shape), self.spec_type)
            assert desc != 0
            self.in_desc_list.append(desc)

            size = acl.get_tensor_desc_size(desc)
            bytes_data = data.tobytes()
            data_ptr = acl.util.bytes_to_ptr(bytes_data)
            host_data_buf = acl.create_data_buffer(data_ptr, size)
            assert host_data_buf != 0
            self.in_host_list.append(host_data_buf)

            dev_ptr, ret = acl.rt.malloc(size, ACL_MEM_MALLOC_HUGE_FIRST)
            assert ret == 0
            ret = acl.rt.memcpy(dev_ptr, size, data_ptr, size, ACL_MEMCPY_HOST_TO_DEVICE)
            assert ret == 0
            self.in_dev_list.append(dev_ptr)
            data_buf = acl.create_data_buffer(dev_ptr, size)
            assert data_buf != 0
            self.in_list.append(data_buf)

        if gen_dataset:
            out_desc = acl.create_tensor_desc(acl_dtype[str(self.type)], list(self.shape), self.spec_type)
            assert out_desc != 0
            self.out_desc_list.append(out_desc)
            self.gen_output_data_set()
        
    def gen_output_data_set(self):
        size = acl.get_tensor_desc_size(self.out_desc_list[0])
        out_dev, ret = acl.rt.malloc(size, ACL_MEM_MALLOC_HUGE_FIRST)
        assert ret == 0
        out_data_buf = acl.create_data_buffer(out_dev, size)
        assert out_data_buf != 0
        self.out_list.append(out_data_buf)
        self.out_dev_list.append(out_dev)

        host_ptr, ret = acl.rt.malloc_host(size)
        assert ret == 0
        self.host_list.append(host_ptr)
    
    def model_update_params(self, op_type):
        ret = acl.op.update_params(op_type, self.in_desc_list, self.out_desc_list, self.attr)
        assert ret == 0
    
    def model_execute(self, op_type="Add"):
        # model execute
        ret = acl.op.execute_v2(op_type, self.in_desc_list, self.in_list, self.out_desc_list,
                                self.out_list, self.attr, self.stream)
        print("ret:",ret)
        unittest.TestCase().assertEqual(ret, 0)
        ret = acl.rt.synchronize_stream(self.stream)
        unittest.TestCase().assertEqual(ret, 0)
        #device to host
        size = acl.get_tensor_desc_size(self.out_desc_list[0])
        acl.rt.memcpy(self.host_list[0], size, self.out_dev_list[0], size, ACL_MEMCPY_DEVICE_TO_HOST)
        bytes_out = acl.util.ptr_to_bytes(self.host_list[0], size)
        data = np.frombuffer(bytes_out, dtype=np.byte)
        return data
    
    def model_op_execute(self, op_type="Add"):
        # model execute
        ret = acl.op.execute(op_type, self.in_desc_list, self.in_list, self.out_desc_list,
                             self.out_list, self.attr, self.stream)
        assert ret == 0
        ret = acl.rt.synchronize_stream(self.stream)
        assert ret == 0

        #device to host
        size = acl.get_tensor_desc_size(self.out_desc_list[0])
        acl.rt.memcpy(self.host_list[0], size, self.out_dev_list[0], size, ACL_MEMCPY_DEVICE_TO_HOST)
        bytes_out = acl.util.ptr_to_bytes(self.host_list[0], size)
        data = np.frombuffer(bytes_out, dtype=np.byte)
        return data

    def np_data_format(self, data, dtype):
        b_arr = data.tobytes()
        arr_2 = np.frombuffer(b_arr, dtype=dtype)
        return arr_2

    def tensor_desc(self):
        size = acl.get_tensor_desc_element_count(self.in_desc_list[0])
        print("size = ", size)
        acl.set_tensor_desc_name(self.in_desc_list[0], "abc")
        print("desc name= ", acl.get_tensor_desc_name(self.in_desc_list[0]))
        fmt = acl.get_tensor_desc_format(self.indesc_list[0])
        print("fmt = ", fmt)
    
    def op_attr(self):
        attr = acl.op.create_attr()
        assert attr != 0

        ret = acl.op.set_attr_bool(attr, "a", 0)
        assert ret == 0
        ret = acl.op.set_attr_int(attr, "b", 1)
        assert ret == 0
        ret = acl.op.set_attr_float(attr, "c", 2.0)
        assert ret == 0
        ret = acl.op.set_attr_string(attr, "d", "123")
        assert ret == 0
        data = [4, 5, 6]
        ret = acl.op.set_attr_list_bool(attr, "e", data)
        assert ret == 0
        data = [1.5, 2.14, 3.11]
        ret = acl.op.set_attr_list_float(attr, "f", data)
        assert ret == 0
        data = [10, 20, 30]
        ret = acl.op.set_attr_list_int(attr, "g", data)
        assert ret == 0
        ret = acl.op.set_attr_list_string(attr, "h", ["1", "2"])
        assert ret == 0
        data = [[10],[20, 30], [40, 50, 60]]
        ret = acl.op.set_attr_list_list_int(attr, "i", data)
        assert ret == 0
        acl.op.destroy_attr(attr)
        return 0

    def exe_with_dynamic_shape(self, op_type):
        out_desc = acl.create_tensor_desc(acl_dtype[str(self.type)], [-1, -1], self.spec_type)
        assert out_desc != 0
        self.out_desc_list.append(out_desc)
        ret = acl.op.infer_shape(op_type, self.in_desc_list, self.in_host_list,
                                 1, self.out_desc_list, self.attr)
        assert ret == 0

        tensor_dims = []
        for i in range(len(self.out_desc_list)):
            dim_nums = acl.get_tensor_desc_num_dims(self.out_desc_list[i])
            dim_size = []
            for j in range(dim_nums):
                dim, ret = acl.get_tensor_desc_dim_v2(self.out_desc_list[i], j)
                assert ret == 0
                if dim == -1:
                    dim_range, ret = acl.get_tensor_desc_dim_range(self.out_desc_list[i], j, 2)
                    assert ret == 0
                    dim = dim_range[1]
                dim_size.append(dim)
            tensor_dims.append(dim_size)
        print("[INFO] infer result: {}".format(tensor_dims))

        self.shape = tensor_dims[0]
        self.gen_output_data_set()
        result = self.model_execute(op_type)
        return result


g_callbackRunFlag = False


class TestOp(unittest.TestCase):

    def setUp(self) -> None:
        pass
    
    def tearDown(self) -> None:
        pass

    @classmethod
    def tearDownClass(cls) -> None:
        ret = acl.rt.reset_device(0)
        if ret:
            print("acl.rt.reset_device! ret:", ret)
            raise AssertionError
        ret = acl.finalize()
        if ret:
            print("acl.finalize failed! ret:", ret)
            raise AssertionError
    
    @classmethod
    def setUpClass(cls) -> None:
        ret = acl.init()
        if ret:
            print("acl.init failed! ret:", ret)
            raise AssertionError
        ret = acl.op.set_model_dir(f"{output_dir}/tmp/pyacl_testcase")
        if ret:
            print("acl.op.set_model_dir failed! ret:", ret)
            raise AssertionError
        ret = acl.rt.set_device(0)
        if ret:
            print("acl.rt.set_device failed! ret:", ret)
            raise AssertionError
    
    def test_op_015_load_op(self):
        """
        test case for loading operator
        """
        np_data = np.fromfile(f"{output_dir}/tmp/pyacl_testcase/0_Add_3_2_8_16_3_2_8_16_3_2_8_16.om", dtype="int8")
        bytes_data = np_data.tobytes()
        buffer = acl.util.bytes_to_ptr(bytes_data)
        np_size = np_data.size 

        ret = acl.op.load(buffer, np_size)
        self.assertEqual(ret, 0)
        
    def test_op_017_normal_op_add(self):
        """
        test case for operator add
        """
        a = np.random.randint(100, size=(8, 16)).astype(np.int32)
        b = np.random.randint(100, size=(8, 16)).astype(np.int32)
        op_handle = AclOp(a, b)
        op_handle.tensor_desc_init()
        res = op_handle.model_execute()
        data = op_handle.np_data_format(res, dtype=np.int32)
        np_res = a + b
        np_out = np.reshape(np_res, (np_res.size,))
        self.assertEqual((data == np_out).all(), True)

if __name__ == "__main__":
    #util.show_growth()
    suite = util.switch_cases(TestOp, "all")
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if result.wasSuccessful():
        exit(0)
exit(1)


