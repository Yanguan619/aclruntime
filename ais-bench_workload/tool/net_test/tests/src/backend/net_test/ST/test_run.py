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
        output_lines = result.stdout.splitlines()
        cur_val = 1024
        pattern = r'\[INFO\]\[HCCL_TEST\]\s*(\d+)\s*\|\s*\d+\.\d+\s*\|\s*\d+\.\d+\s*\|\s*(success|failed|NULL)'
        for line in output_lines:
            match = re.search(pattern, line)
            if match:
                value = int(match.group(1))
                result = match.group(2)
                self.assertEqual(value, cur_val, f"data_size no match")
                self.assertEqual(result, 'success', f"check_result is not success")
                cur_val *= 2

        self.assertEqual(cur_val, 262144 * 2, "run error")


if __name__ == "__main__":
    unittest.main()