import unittest
import acl
import utils as util


class TestLog(unittest.TestCase):
    
    def setUp(self) -> None:
        pass
    
    def tearDownClass(cls) -> None:
        pass
    
    @classmethod
    def tearDownClass(cls) -> None:
        pass
    
    @classmethod
    def setUpClass(cls):
        pass
    
    def test_log_001_normal(self):
        """
        test case for acl app_log, recording debug/info/warning/error logs
        """
        a = [1, 2, 3]
        b = (3, 2 ,1)
        c = {4, 5, 6}
        d = {"a": 1, "b":2 ,"c": 3}
        e= 1.1111
        f = 1111111111
        g = "1111111111"
        # log level = 0, debug
        acl.app_log(0, "a = {}, b = {}, c = {}, d = {}, e = {}, f = {}, g = {}".format(a,b,c,d,e,f,g))
        # log level = 1, info
        acl.app_log(1, "a = {}, b = {}, c = {}, d = {}, e = {}, f = {}, g = {}".format(a,b,c,d,e,f,g))
        # log level = 2, warning
        acl.app_log(2, "a = {}, b = {}, c = {}, d = {}, e = {}, f = {}, g = {}".format(a,b,c,d,e,f,g))
        # log level = 3, error
        acl.app_log(3, "a = {}, b = {}, c = {}, d = {}, e = {}, f = {}, g = {}".format(a,b,c,d,e,f,g))

    def test_log_002_err_parameter(self):
        """
        test case for acl app_log with invalid parameter
        """
        # log level = 3, error
        params = {'type': 'is',
                  'params': [(), ('', ''), (123, 321)]}
        self.assertEqual(util.params_check(self, params, acl.app_log), 0)
    
if __name__ == "__main__":

    suite = util.switch_cases(TestLog, "all")
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if result.wasSuccessful():
        exit(0)
exit(1)
    