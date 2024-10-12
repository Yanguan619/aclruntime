import subprocess
import unittest
import re
import socket

class TestNetBench(unittest.TestCase):

    def setUp(self):
        pass
 

    def tearDown(self):
        pass


    @classmethod
    def setUpClass(cls):
        pass


    @classmethod
    def tearDownClass(cls):
        pass


    def test_run_backend_net_test(self):
        server_ip = socket.gethostbyname(socket.gethostname())
        rank_size = "8"
        npus = "8"
        print(server_ip)
        command = [
            "python3", "-m", "ais_bench.backend.net_test",
            "--rank_size", rank_size,
            "--server_ip", server_ip,
            "--server_port", "12235",
            "--node_id", "0",
            "--npus", npus,
            "--minbytes", "1K",
            "--maxbytes", "256K"
        ]
        
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.assertEqual(result.returncode, 0, f"命令执行失败: {result.stderr}")
        
        output_lines = result.stdout.splitlines()

        cur_val = 1024
        pattern = r'\[INFO\]\[HCCL_TEST\]\s*(\d+)\s*\|\s*\d+\.\d+\s*\|\s*\d+\.\d+\s*\|\s*(success|failure)'
        for line in output_lines:
            print(f"Processing line: {line}") 
            match = re.search(pattern, line)
            if match:
                value = int(match.group(1))
                result = match.group(2)
                self.assertEqual(value, cur_val, f"匹配失败")
                self.assertEqual(result, 'success', f"匹配失败")
                cur_val *= 2

        self.assertEqual(cur_val, 262144 * 2, "未匹配到所有预期值")


if __name__ == "__main__":
    unittest.main()