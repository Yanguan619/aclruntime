# encoding: utf-8
import unittest
import time
import threading
import acl
import utils as util



ACL_EVENT_TIME_LINE = 0x0000008
ACL_EVENT_RECORD_STATUS_NOT_READY = 0
ACL_EVENT_RECORD_STATUS_COMPLETE = 1

g_callbackRunFlag = True


def launch_callback_fun_1(args_list):
    for i in range(3):
        print("lanuch_callback_fun 1")
        print(args_list)


def launch_callback_fun_2(args_list):
    for i in range(3):
        print("lanuch_callback_fun 2")
        print(args_list)
        

def callback_thr_func(args_list):
    print("[callbacl_thr_func] args = ", args_list[0], args_list[1])
    timeout = args_list[1]
    
    print("[callback_thr_func] g_callbackRunFlag = ", g_callbackRunFlag, timeout)
    
    while g_callbackRunFlag is True:
        print("[callback_thr_func] g_callbackRunFlag = ", g_callbackRunFlag)
        ret = acl.rt.process_report(timeout)
        print("[INFO] process_report ret = ", ret)
    
    print("[INFO] end")


class TestEvent(unittest.TestCase):
    
    def setUp(self):
        
        pass
    
    def tearDown(self):
        
        pass

    @classmethod
    def tearDownClass(cls):
        ret = acl.finalize()
        assert ret == 0
    
    @classmethod
    def setUpClass(cls):
        ret = acl.init()
        assert ret == 0
        
    def test_event_001_normal(self):
        """
        test case for creating and destroying event
        :return:
        """
        ret = acl.rt.set_device(0)
        self.assertEqual(ret, 0)
        et, ret = acl.rt.create_event()
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_event(et)
        self.assertEqual(ret, 0)
    
    def test_event_006_callback(self):
        """
        test case for launching a callback function to do soming
        1.init resource : create_contest create_stream
        2.start a task by starting a thread
          the thread triggers callback processing by calling process_report
        3.register the thread tor handle the callback function
        4.launch a callback function
        5.unresgistering a thread
        6.free reasources
        :return:
        """
        ret = acl.rt.set_device(0)
        self.assertEqual(ret, 0)
        stream, ret = acl.rt.create_stream()
        self.assertEqual(ret, 0)
        
        timeout = 1000
        global g_callbackRunFlag
        g_callbackRunFlag = True
        args_list = [g_callbackRunFlag, timeout]
        thr_id, ret = acl.util.start_thread(callback_thr_func,args_list)
        self.assertEqual(ret,0)
        
        ret = acl.rt.subscribe_report(thr_id, stream)
        self.assertEqual(ret, 0)
        
        ret = acl.rt.launch_callback(launch_callback_fun_1, ["zzq", "qzz"], 1, stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.launch_callback(launch_callback_fun_2, ["zzq", "qzz"], 1, stream)
        self.assertEqual(ret, 0)
        
        ret = acl.rt.synchronize_stream(stream)
        self.assertEqual(ret, 0)
        
        ret = acl.rt.subscribe_report(thr_id, 0)
        self.assertEqual(ret, 0)
        
        ret = acl.rt.launch_callback(launch_callback_fun_1, ["zzq", "qzz"], 1, 0)
        self.assertEqual(ret, 0)
        ret = acl.rt.launch_callback(launch_callback_fun_2, ["zzq", "qzz"], 1, 0)
        self.assertEqual(ret, 0)
        
        ret = acl.rt.synchronize_stream(0)
        self.assertEqual(ret, 0)
        
        g_callbackRunFlag = False
        
        ret =acl.rt.unsubscribe_report(thr_id, stream)
        self.assertEqual(ret, 0)
        
        ret =acl.rt.unsubscribe_report(thr_id, 0)
        self.assertEqual(ret, 0)
        
        ret =acl.util.stop_thread(thr_id)
        self.assertEqual(ret, 0)
        
        ret = acl.rt.destroy_stream(stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.reset_device(0)
        self.assertEqual(ret, 0)
        
    def test_event_0008_multi_streams(self):
        """
        test case for acl synchroniztion waiting interface with mulit-streams
        1. set device, create stream1, stream2, event1, event2
        2. record event1 and event2 to the stream1 handle
        3. call stream_waitevent function  to block current stream, waiting for the event finished
        4. query the lapsed time between the two event
        5. free resources
        :return:
        """
        device_id = 0
        
        context, ret = acl.rt.create_context(device_id)
        stream, ret = acl.rt.create_stream()
        self.assertEqual(ret, 0)
        stream_2, ret = acl.rt.create_stream()
        self.assertEqual(ret, 0)
        event_1, ret = acl.rt.create_event()
        self.assertEqual(ret, 0)
        event_2, ret = acl.rt.create_event()
        self.assertEqual(ret, 0)
        ret = acl.rt.record_event(event_1, stream)
        self.assertEqual(ret, 0)
        time.sleep(0.005)
        ret = acl.rt.record_event(event_2, stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.synchronize_event(event_1)
        self.assertEqual(ret, 0)
        ret = acl.rt.stream_wait_event(stream_2, event_1)
        self.assertEqual(ret, 0)
        status, ret = acl.rt.query_event_wait_status(event_1)
        self.assertEqual(ret, 0)
        ret = acl.rt.synchronize_event(event_2)
        self.assertEqual(ret, 0)
        status, ret = acl.rt.query_event_status(event_2)
        self.assertEqual(ret, 0)
        self.assertEqual(status, ACL_EVENT_RECORD_STATUS_COMPLETE)
        
        ret = acl.rt.synchronize_stream(stream)
        self.assertEqual(ret, 0)
        ms, ret = acl.rt.event_elapsed_time(event_1, event_2)
        self.assertEqual(ret, 0)
        self.assertLessEqual(ms, 10)
        
        ret = acl.rt.destroy_event(event_1)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_event(event_2)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_stream(stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_stream(stream_2)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_context(context)
        self.assertEqual(ret, 0)

    def test_event_009_elapsed_time(self):
        """
        test case for acl event elapsed_time
        :return:
        """
        device_id = 0
        context, ret =acl.rt.create_context(device_id)
        self.assertEqual(ret, 0)
        stream, ret =acl.rt.create_stream()
        self.assertEqual(ret, 0)
        event_1, ret =acl.rt.create_event_with_flag(ACL_EVENT_TIME_LINE)
        self.assertEqual(ret, 0)
        event_2, ret = acl.rt.create_event_with_flag(ACL_EVENT_TIME_LINE)
        self.assertEqual(ret, 0)
        ret = acl.rt.record_event(event_1, stream)
        self.assertEqual(ret, 0)
        # sleep 2s to simulate the computational task
        time.sleep(2)
        ret = acl.rt.record_event(event_2, stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.synchronize_stream(stream)
        self.assertEqual(ret, 0)
        ms, ret = acl.rt.event_elapsed_time(event_1,event_2)
        self.assertEqual(ret, 0)
        print("[INFO] ms = {}".format(ms))
        
        ret = acl.rt.destroy_event(event_1)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_event(event_2)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_stream(stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_context(context)
        self.assertEqual(ret, 0)
    
    def launch_callback_with_pythreading(self, blocked, sleep_time, blocked_time):
        """
        1、创建回调处理线程
        2、包装一个符合回调函数格式的sleep函数
        3、调用launch_callback 组色参数设置为blocked
        4、验证阻塞时间是否为blocked_time
        5、关闭线程
        """
        ret = acl.rt.set_device(0)
        self.assertEqual(ret, 0)
        stm, ret = acl.rt.create_stream()
        self.assertEqual(ret, 0)
        thd_flag = True
        
        def process_report_loop():
            # 回调函数处理线程
            max_time = 15 # 最长运行15s
            t = time.perf_counter()
            while thd_flag and time.perf_counter() - t < max_time:
                # 每200ms重新调用process_report
                acl.rt.process_report(200)
        
        thd = threading.Thread(target=process_report_loop)
        thd.start()
        
        ret = acl.rt.subscribe_report(thd.ident, stm)
        self.assertEqual(0, ret)
        
        def sleep_cbk(t):
            for i in t:
                time.sleep(i)
        
        ret = acl.rt.synchronize_stream(stm)
        self.assertEqual(ret, 0)
        st = time.perf_counter()
        # 调用回调函数阻塞stllep_time秒
        ret = acl.rt.launch_callback(sleep_cbk, [sleep_time], blocked, stm)
        self.assertEqual(ret, 0)
        ret = acl.rt.synchronize_stream(stm)
        self.assertAlmostEqual(blocked_time, time.perf_counter() -st, delta=0.01)
        self.assertEqual(ret, 0)
        
        # 将thd_flag设置为 false 关闭回调线程
        thd_flag = False
        self.assertEqual(ret, 0)
        
        thd.join()
        
        ret = acl.rt.unsubscribe_report(thd.ident, stm)
        
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_stream(stm)
        self.assertEqual(ret, 0)
        ret = acl.rt.reset_device(0)
        self.assertEqual(ret, 0)
    
    def test_event_013_launch_callback_with_pythread_blocaked(self):
        """
        测试使用python线程库作为subscribe_report的回调线程,并验证 launch_callback blocked参数设置为1时回调函数是否能够阻塞stream
        阻塞stream场景下blocked_stream 和 stream 相等为2s
        """
        self.launch_callback_with_pythreading(1, 2, 2)
    
    def test_event_017_ex_event(self):
        """
        test case for acl event elapsed_time
        :return:
        """
        device_id = 0
        context, ret =acl.rt.create_context(device_id)
        self.assertEqual(ret, 0)
        stream, ret =acl.rt.create_stream()
        self.assertEqual(ret, 0)
        event_1, ret =acl.rt.create_event_ex_with_flag(ACL_EVENT_TIME_LINE)
        self.assertEqual(ret, 0)
        event_2, ret = acl.rt.create_event_ex_with_flag(ACL_EVENT_TIME_LINE)
        self.assertEqual(ret, 0)
        ret = acl.rt.record_event(event_1, stream)
        self.assertEqual(ret, 0)
        # sleep 2s to simulate the computational task
        time.sleep(2)
        ret = acl.rt.record_event(event_2, stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.synchronize_stream(stream)
        self.assertEqual(ret, 0)
        ms, ret = acl.rt.event_elapsed_time(event_1,event_2)
        self.assertEqual(ret, 0)
        print("[INFO] ms = {}".format(ms))
        
        ret = acl.rt.destroy_event(event_1)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_event(event_2)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_stream(stream)
        self.assertEqual(ret, 0)
        ret = acl.rt.destroy_context(context)
        self.assertEqual(ret, 0)
        
if __name__ == "__main__":
    suite = util.switch_cases(TestEvent, "all")
    unittest.TextTestRunner(verbosity=2).run(suite)
    