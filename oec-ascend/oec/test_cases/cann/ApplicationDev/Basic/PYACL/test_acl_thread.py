import time
import unittest
import acl
import utils as util

g_callbackFunFlag = True
g_context = 0
g_timeout = 0


def callback_thr_func(args_list):
    ctx = args_list[0]
    timeout = args_list[1]
    if ctx != g_context:
        raise Exception("{} != {}".format(ctx, g_context))
    if timeout != g_timeout:
        raise Exception("{} != {}".format(timeout, g_timeout))
    
    ret = acl.rt.set_context(ctx)
    print(f"acl.rt.set_context(ctx) ret = {ret}")
    
    start = time.time()
    while g_callbackFunFlag is True:
        run_time = time.time() - start
        if run_time > 10:
            break
        time.sleep(1)
        ret = acl.rt.process_report(timeout)
        print(f"acl.rt.process_report(timeout) ret = {ret}")
    localtime = time.asctime(time.localtime(time.time()))
    print("[INFO] after process_report", g_callbackFunFlag, localtime)
    

class TestThread(unittest.TestCase):
    
    def setUp(self) -> None:
        pass
    
    def tearDownClass(cls) -> None:
        pass
    
    @classmethod
    def tearDownClass(cls) -> None:
        ret = acl.rt.reset_device(0)
        assert ret == 0
        ret = acl.finalize()
        assert ret == 0
    
    @classmethod
    def setUpClass(cls):
        ret = acl.init()
        assert ret == 0
        ret == acl.rt.set_device(0)
        assert ret == 0
        
    def test_thread_001_normal(self):
        """
        test case for starting a c thread
        """
        
        global g_callbackFunFlag
        global g_context
        global g_timeout
        
        g_callbackFunFlag = True
        ctx, ret = acl.rt.create_context(0)
        self.assertEqual(ret, 0)
        g_context = ctx
        timeout = 1000
        g_timeout = timeout
        
        args_list = [ctx, timeout]
        callback_thr_id, ret = acl.util.start_thread(callback_thr_func, args_list)
        self.assertEqual(ret, 0)
        g_callbackFunFlag = False
        
        ret = acl.util.stop_thread(callback_thr_id)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_context(ctx)
        self.assertEqual(ret, 0)
    
    def test_thread_002_two_thread(self):
        """
        test case for starting two c threads
        """
        global g_callbackFunFlag
        global g_context
        global g_timeout
        
        g_callbackFunFlag = True
        ctx, ret = acl.rt.create_context(0)
        self.assertEqual(ret, 0)
        g_context = ctx
        timeout = 1000
        g_timeout = timeout
        
        args_list = [ctx, timeout]
        callback_thr_id_1, ret = acl.util.start_thread(callback_thr_func, args_list)
        self.assertEqual(ret, 0)
        callback_thr_id_2, ret = acl.util.start_thread(callback_thr_func, args_list)
        self.assertEqual(ret, 0)
        g_callbackFunFlag = False
        
        ret = acl.util.stop_thread(callback_thr_id_1)
        self.assertEqual(ret, 0)
        ret = acl.util.stop_thread(callback_thr_id_2)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_context(ctx)
        self.assertEqual(ret, 0)
    
    def test_thread_003_error(self):
        """
        test case for acl start_thread with no paramter
        """
        
        try:
            acl.util.start_thread()
        except Exception as e:
            self.assertIn(e.__str__(), ["function takes exactly 2 arguments (0 given)"])
        else:
            self.fail("Expected exception not raised")
    
    def test_thread_004_error(self):
        """
        test case for acl start_thread with invalid parmeter
        """
        
        try:
            acl.util.start_thread('', '')
        except Exception as e:
            self.assertIn(e.__str__(), ["function takes exactly 2 arguments (0 given)", "parameter must be callable"])
    
    def test_thread_005_error_param_stop_thread(self):
        """
        test case for acl stop_thread with invalid parmeter
        """
        
        params = {
            'type': 'k',
            'params': [(), ('',)]
        }
        self.assertEqual(util.params_check(self, params, acl.util.stop_thread), 0)

if __name__ == "__main__":

    suite = util.switch_cases(TestThread, "all")
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if result.wasSuccessful():
        exit(0)
exit(1)
    