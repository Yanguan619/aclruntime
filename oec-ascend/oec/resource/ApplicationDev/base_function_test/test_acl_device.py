# encoding: utf-8
# 版权所有 (C) 华为技术有限公司 2022-2023
import unittest
import logging


import utils as util
import acl

ACL_DEVICE = 0
ACL_HOST = 1
ACL_RT_OVERFLOW_MODE_SATURATION = 0
ACL_RT_OVERFLOW_MODE_INFNAN = 1
ACL_RT_OVERFLOW_MODEL_UNDEF=2


class TestDevice(unittest.TestCase):
    
    @classmethod
    def tearDownClass(cls):
        #after all test
        pass
    
    @classmethod
    def setUpClass(cls):
        # before all test
        pass
    
    def setUp(self):
        # before one test
        pass
    
    def tearDown(self):
        # after one test
        pass
    
    def test_device_001_normal(self):
        """
        test case for setting and restting device
        1. set device 0
        2. get and check device id
        3. reset device 0
        """
    
    def test_device_007_get_device_utilization_rate(self):
        """
        获取device的cube, aicpu, vector core单元的使用率
        1、获取环境上的npu数量
        2、获取每个npu的使用率
        3、检查获取到的使用率是否包含所有必要的字段
        """
        n, ret =acl.rt.get_device_count()
        self.assertEqual(ret, 0)
        tmp = {
            'cube_utilization': 0,
            'vector_utilization': 0,
            'aicpu_utilization': 0,
            'memory_utilization': 0,
            'utilization_extend': 0
        }
        for i in range(n):
            rst, ret = acl.rt.get_device_utilization_rate(i)
            self.assertEqual(ret, 0)
            for key in tmp:
                self.assertIn(key, rst)
    
    def test_device_009_query_device_status(self):
        """
        test device status
        1. get device count
        2. query status for each device
        3. check statis is ok for each device
        """
        n, ret = acl.rt.get_device_count()
        self.assertEqual(ret, 0)
        for i in range(n):
            status, ret = acl.rt.query_device_status(i)
            self.assertEqual(ret, 0)
            self.assertEqual(status, 0)
            
    def test_device_010_peek_at_last_error(self):
        """
        test device peek at last error
        1. make a mistake to rasie error
        2. peek last error
        3. check error is not cleared
        """
        ret = acl.rt.set_device(-1)
        self.assertNotEqual(ret, 0)
        
        #测试捕获错误码
        ret = acl.rt.peek_at_last_error(0)
        self.assertNotEqual(ret, 0)
        
        # 测试错误吗没有被清空
        ret = acl.rt.peek_at_last_error(0)
        self.assertNotEqual(ret, 0)
        
    def test_device_011_synchronize_device_with_timeout(self):
        """
        test synchronize device with timeout
        """
        ret = acl.rt.set_device(ACL_DEVICE)
        self.assertEqual(ret, 0)
        
        ret = acl.rt.synchronize_device_with_timeout(5)
        self.assertEqual(ret, 0)
    
    def test_device_017_reset_device_force(self):
        """
        test reset device force
        """
        ret = acl.rt.set_device(ACL_DEVICE)
        self.assertEqual(ret, 0)
        ret = acl.rt.reset_device(ACL_DEVICE)
        self.assertEqual(ret, 0)
        ret = acl.rt.set_device(ACL_DEVICE)
        self.assertEqual(ret, 0)
        ret = acl.rt.reset_device_force(ACL_DEVICE)
        self.assertEqual(ret, 0)

if __name__ == "__main__":
    suite = util.switch_cases(TestDevice, "all")
    unittest.TextTestRunner(verbosity=2).run(suite)
    