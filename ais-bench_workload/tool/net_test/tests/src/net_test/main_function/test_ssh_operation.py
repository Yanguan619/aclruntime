import unittest
import paramiko
from unittest.mock import patch, Mock
from ais_bench.net_test.ssh.ssh_operation import (ssh_client_connect, remote_exec, remote_exec_file_check, remote_put
    )
from ais_bench.net_test.sub_module.base_sub_module import NodeInfo


class FakeBufferedFile:
    def read(self):
        return b"1"
    def readline(self, n):
        return "11"


class TestCheckFuncUtils(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    @patch("paramiko.SSHClient.connect")
    def test_ssh_client_connect(self, mock_ssh_client):
        ssh_client = paramiko.SSHClient()
        node_info = NodeInfo("XX", 1, "A", 123)
        mock_ssh_client.side_effect = Exception('An error occurred')
        with self.assertRaisesRegex(RuntimeError, "ssh connect use ssh key"):
            ssh_client_connect(ssh_client, node_info, "./")

        mock_ssh_client.side_effect = None
        with self.assertRaisesRegex(FileExistsError, "ssh_key_path not offered"):
            ssh_client_connect(ssh_client, node_info, "")

    @patch("paramiko.SSHClient.close")
    @patch("ais_bench.net_test.ssh.ssh_operation.ssh_client_connect")
    @patch("paramiko.SSHClient.exec_command")
    def test_remote_exec_file_check(self, mock_exec, mock_connect, mock_close):
        node_info = NodeInfo("XX", 1, "A", 123)
        mock_exec.return_value = ("1", FakeBufferedFile(), FakeBufferedFile())

        mock_exec.side_effect = Exception('An error occurred')
        with self.assertRaisesRegex(RuntimeError, "exec command:"):
            remote_exec_file_check("./", node_info, "./")

        mock_exec.side_effect = None
        mock_read = Mock(spec=FakeBufferedFile)
        mock_read.read.return_value = b"dd"
        with self.assertRaisesRegex(RuntimeError, "remote check file failed! error log"):
            remote_exec_file_check("./", node_info, "./")

    @patch("paramiko.SSHClient.close")
    @patch("ais_bench.net_test.ssh.ssh_operation.console_origin")
    @patch("ais_bench.net_test.ssh.ssh_operation.ssh_client_connect")
    @patch("paramiko.SSHClient.exec_command")
    def test_remote_exec(self, mock_exec, mock_connect, mock_console, mock_close):
        node_info = NodeInfo("XX", 1, "A", 123)
        mock_exec.return_value = ("1", FakeBufferedFile(), FakeBufferedFile())

        mock_exec.side_effect = Exception('An error occurred')
        with self.assertRaisesRegex(RuntimeError, "exec command:"):
            remote_exec_file_check("./", node_info, "./")

        mock_exec.side_effect = None
        mock_read = Mock(spec=FakeBufferedFile)
        mock_read.read.return_value = b"ERROR"
        mock_read.readline.return_value = "1111111111"
        with self.assertRaisesRegex(RuntimeError, "failed, error log from node:"):
            remote_exec("./", node_info, "./")

    @patch("paramiko.SSHClient.close")
    @patch("scp.SCPClient.close")
    @patch("scp.SCPClient.put")
    @patch("scp.SCPClient")
    @patch("ais_bench.net_test.ssh.ssh_operation.ssh_client_connect")
    def test_remote_put(self, mock_connect, mock_scp, mock_scp_put, mock_scp_close, mock_ssh_close):
        node_info = NodeInfo("XX", 1, "A", 123)
        mock_scp.side_effect = Exception('An error occurred')
        with self.assertRaisesRegex(RuntimeError, "open trans_client failed"):
            remote_put(1, node_info, "./", "./", "./")

        mock_scp.side_effect = None
        mock_scp_put.side_effect = Exception('An error occurred')
        with self.assertRaisesRegex(RuntimeError, "to dst_path:"):
            remote_put(1, node_info, "./", "./", "./")


if __name__ == "__main__":
    unittest.main()