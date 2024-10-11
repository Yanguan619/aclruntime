import unittest  
import stat  
from unittest.mock import patch, MagicMock
from ais_bench.backend.net_test.common.consts import RET

from ais_bench.backend.net_test.launch_run_node import (  
    launch_run_node, construct_command_lists, multiprocess_run, parse_result,  
    generate_rank_id_list, get_rank_related_cmd_list, run_hccl_test_exec_command
)  


class TestLaunchRunNode(unittest.TestCase):

    def setUp(self):
        pass
 

    def tearDown(self):
        pass
    

    @classmethod
    def tearDownClass(cls):
        pass


    @classmethod
    def setUpClass(cls):
        pass


    @patch('ais_bench.backend.net_test.launch_run_node.construct_command_lists')
    @patch('ais_bench.backend.net_test.launch_run_node.multiprocess_run')
    @patch('ais_bench.backend.net_test.launch_run_node.parse_result')
    def test_launch_run_node_success(self, mock_parse_result, mock_multiprocess_run, mock_construct_command_lists):
        mock_construct_command_lists.return_value = [  
            ["hccl_test/bin/all_reduce_test", "--rank_id", "0", "-f"],  
            ["hccl_test/bin/all_reduce_test", "--rank_id", "1", "-f"],  
        ]  
        mock_multiprocess_run.return_value = [(RET.SUCCESS, ''), (RET.SUCCESS, '')]
        mock_parse_result.return_value = RET.SUCCESS
        
        args = MagicMock()
        args.npus = 2

        result = launch_run_node(args)
        self.assertEqual(result, RET.SUCCESS)
        mock_construct_command_lists.assert_called_once_with(args)
        mock_multiprocess_run.assert_called_once_with(args.npus, mock_construct_command_lists.return_value)
        mock_parse_result.assert_called_once_with(mock_multiprocess_run.return_value, args)


    @patch('ais_bench.backend.net_test.launch_run_node.construct_command_lists')
    @patch('ais_bench.backend.net_test.launch_run_node.multiprocess_run')
    @patch('ais_bench.backend.net_test.launch_run_node.parse_result')
    def test_launch_run_node_failure(self, mock_parse_result, mock_multiprocess_run, mock_construct_command_lists):
        mock_construct_command_lists.return_value = [  
            ["hccl_test/bin/all_reduce_test", "--rank_id", "0", "-f"],  
            ["hccl_test/bin/all_reduce_test", "--rank_id", "1", "-f"],  
        ]  
        mock_multiprocess_run.return_value = [(RET.FAILED, 'Cmd failed!'), (RET.SUCCESS, '')]
        mock_parse_result.return_value = RET.FAILED
        
        args = MagicMock()
        args.npus = 2
        result = launch_run_node(args)
        self.assertEqual(result, RET.FAILED)
        mock_construct_command_lists.assert_called_once_with(args)
        mock_multiprocess_run.assert_called_once_with(args.npus, mock_construct_command_lists.return_value)
        mock_parse_result.assert_called_once_with(mock_multiprocess_run.return_value, args)


    @patch('os.path.join')
    @patch('os.path.exists')
    @patch('os.stat')
    @patch('ais_bench.backend.net_test.launch_run_node.generate_rank_id_list')
    @patch('ais_bench.backend.net_test.launch_run_node.get_rank_related_cmd_list')
    @patch('ais_bench.net_test.security.file_checker.check_linux_file_path')
    def test_construct_command_lists(self, mock_check_linux_file_path, mock_get_rank_related_cmd_list, mock_generate_rank_id_list, mock_stat, mock_exists, mock_join):
        mock_join.return_value = 'tools/ais-bench_workload/tool/net_test/ais_bench/backend/net_test/hccl_test/bin/all_reduce_test'
        mock_exists.return_value = True
        mock_stat.return_value = MagicMock(st_mode=stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR)
        mock_generate_rank_id_list.return_value = ['0']
        mock_get_rank_related_cmd_list.return_value = []

        args = MagicMock()
        args.op_task = 'all_reduce_test'
        args.rank_id = '0'

        cmd_lists = construct_command_lists(args)
        mock_check_linux_file_path.assert_called_once()
        expected_cmd_lists = [['tools/ais-bench_workload/tool/net_test/ais_bench/backend/net_test/hccl_test/bin/all_reduce_test', '--rank_id', '0']]
        self.assertEqual(cmd_lists, expected_cmd_lists)


    @patch('subprocess.Popen')
    def test_run_hccl_test_exec_command_success(self, mock_popen):
        mock_process = MagicMock()
        mock_process.stdout.readline = MagicMock(side_effect=[b'output line 1\n', b'', b''])
        mock_process.communicate.return_value = (b'', b'')
        mock_process.wait.return_value = RET.SUCCESS
        mock_popen.return_value = mock_process

        result = run_hccl_test_exec_command(['cmd1', 'cmd2'])
        self.assertEqual(result, (RET.SUCCESS, ""))


    @patch('subprocess.Popen')
    def test_run_hccl_test_exec_command_failure(self, mock_popen):
        mock_process = MagicMock()
        mock_process.stdout.readline = MagicMock(side_effect=[b'', b''])
        mock_process.communicate.return_value = (b'', b'error occurred')
        mock_process.wait.return_value = RET.FAILED
        mock_popen.return_value = mock_process

        result = run_hccl_test_exec_command(['cmd1', 'cmd2'])
        self.assertEqual(result, (RET.FAILED, "Cmd ['cmd1', 'cmd2'] failed! error log: error occurred"))

    @patch('subprocess.Popen')
    def test_multiprocess_run(self, mock_popen):
        mock_process = MagicMock()
        mock_process.stdout.readline = MagicMock(side_effect=[b'output line 1\n', b'', b''])
        mock_process.communicate.return_value = (b'', b'')
        mock_process.wait.return_value = RET.SUCCESS
        mock_popen.return_value = mock_process

        command_lists = [['cmd1', 'test1'], ['cmd2', 'test2']]
        results = multiprocess_run(2, command_lists)
        self.assertEqual(results, [(RET.SUCCESS, ""), (RET.SUCCESS, "")])


    def test_generate_rank_id_list(self):
        class Args:
            def __init__(self, node_id, npus):
                self.node_id = node_id
                self.npus = npus

        args = Args(node_id=2, npus=4)
        expected_rank_id_list = [8, 9, 10, 11]
        result = generate_rank_id_list(args)
        self.assertEqual(result, expected_rank_id_list)

    @patch('logging.Logger.error')
    def test_parse_result_success(self, mock_error):
        class Args:
            def __init__(self, node_id, npus):
                self.node_id = node_id
                self.npus = npus

        args = Args(node_id=1, npus=3)
        results = [(RET.SUCCESS, ""), (RET.SUCCESS, ""), (RET.SUCCESS, "")]
        
        result = parse_result(results, args)
        self.assertEqual(result, RET.SUCCESS)
        mock_error.assert_not_called()

    @patch('logging.Logger.error')
    def test_parse_result_failure(self, mock_error):
        class Args:
            def __init__(self, node_id, npus):
                self.node_id = node_id
                self.npus = npus

        args = Args(node_id=1, npus=3)
        results = [(RET.FAILED, "Cmd ['cmd1', 'cmd2'] failed! error log: error occurred"), (RET.SUCCESS, ""), (RET.SUCCESS, "")]
        
        result = parse_result(results, args)
        self.assertEqual(result, RET.FAILED)
        mock_error.assert_called_once_with("rank_id:3, device id:0, run failed! error info:Cmd ['cmd1', 'cmd2'] failed! error log: error occurred")


    def test_get_rank_related_cmd_list_with_valid_args(self):  
        mock_args = MagicMock()  
        mock_args.get_rank_related_args_dict.return_value = {  
            "--rank": 1,  
            "--stepbytes": 1024,  
            "--size": 512  
        }  
          
        result = get_rank_related_cmd_list(mock_args)  
        expected = ["--rank", "1", "--stepbytes", "1024", "--size", "512"]  
        self.assertListEqual(result, expected)  


    def test_get_rank_related_cmd_list_with_zero_stepbytes(self):  
        mock_args = MagicMock()  
        mock_args.get_rank_related_args_dict.return_value = {  
            "--rank": 1,  
            "--stepbytes": 0,  
            "--size": 512  
        }  
          
        result = get_rank_related_cmd_list(mock_args)  
        expected = ["--rank", "1", "--size", "512"]  
        self.assertListEqual(result, expected)  
      

    def test_get_rank_related_cmd_list_with_empty_args(self):  
        mock_args = MagicMock()  
        mock_args.get_rank_related_args_dict.return_value = {}  
        result = get_rank_related_cmd_list(mock_args)  

        expected = []  

        self.assertListEqual(result, expected)  
  
if __name__ == '__main__':  
    unittest.main()