import unittest
import paramiko
from unittest.mock import patch
from ais_bench.net_test.ssh.ssh_operation import (ssh_client_connect, remote_exec, remote_exec_file_check, remote_put
    )
from ais_bench.net_test.sub_module.base_sub_module import NodeInfo

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
    @patch("paramiko.ChannelFile.read")
    @patch("ais_bench.net_test.ssh.ssh_operation.ssh_client_connect")
    @patch("paramiko.SSHClient.exec_command")
    def test_remote_exec_file_check(self, mock_exec, mock_connect, mock_read, mock_close):
        node_info = NodeInfo("XX", 1, "A", 123)
        stdout = paramiko.ChannelFile()
        stderr = paramiko.ChannelFile()
        mock_exec.return_value = tuple[None, stdout, stderr]

        mock_exec.side_effect = Exception('An error occurred')
        with self.assertRaisesRegex(RuntimeError, "exec command:"):
            remote_exec_file_check("./", node_info, "./")

        mock_exec.side_effect = None
        mock_read.return_value = b"1111"
        with self.assertRaisesRegex(RuntimeError, "remote check file failed! error log"):
            remote_exec_file_check("./", node_info, "./")

    @patch("paramiko.SSHClient.close")
    @patch("paramiko.ChannelFile.readlines")
    @patch("paramiko.ChannelFile.read")
    @patch("ais_bench.net_test.ssh.ssh_operation.console_origin")
    @patch("ais_bench.net_test.ssh.ssh_operation.ssh_client_connect")
    @patch("paramiko.SSHClient.exec_command")
    def test_remote_exec(self, mock_exec, mock_connect, mock_console, mock_read, mock_readlines, mock_close):
        node_info = NodeInfo("XX", 1, "A", 123)
        stdout = paramiko.ChannelFile()
        stderr = paramiko.ChannelFile()
        mock_exec.return_value = tuple[None, stdout, stderr]

        mock_exec.side_effect = Exception('An error occurred')
        with self.assertRaisesRegex(RuntimeError, "exec command:"):
            remote_exec_file_check("./", node_info, "./")

        mock_readlines.return_value = ""
        mock_read.return_value = b"ERROR"
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