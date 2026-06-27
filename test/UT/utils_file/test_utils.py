import unittest
from unittest.mock import patch
import os
import numpy as np
from ais_bench.infer.common.utils import (
    list_split,
    list_share,
    natural_sort,
    get_fileslist_from_dir,
    get_file_datasize,
    get_file_content,
    get_ndata_fmt,
    save_data_to_files,
    create_fake_file_name,
    get_dump_relative_paths,
    get_msprof_bin_path,
    get_msaccucmp_path,
    make_dirs,
    create_tmp_acl_json,
    convert_helper,
    move_subdir,
    str_to_uint,
)


class MockMsOpen:
    def __init__(self, content):
        self.content = content

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

    def read(self):
        return self.content


class TestYourModule(unittest.TestCase):
    def test_list_split(self):
        # Test list splitting with padding
        input_list = [1, 2, 3, 4, 5]
        padding_file = "pad"
        result = list(list_split(input_list, 2, padding_file))
        expected = [[1, 2], [3, 4], [5, "pad"]]
        self.assertEqual(result, expected)

    def test_list_share(self):
        # Test list sharing
        input_list = [1, 2, 3, 4, 5, 6, 7, 8, 9]
        result = list(list_share(input_list, 3, 2, 1))
        expected = [[1, 2, 3], [4, 5], [6, 7]]
        self.assertEqual(result, expected)

    def test_natural_sort(self):
        # Test natural sorting
        input_list = ["file10", "file2", "file1"]
        result = natural_sort(input_list)
        expected = ["file1", "file2", "file10"]
        self.assertEqual(result, expected)

    @patch("os.listdir")
    @patch("ais_bench.infer.common.utils.FileStat")
    def test_get_fileslist_from_dir(self, mock_file_stat, mock_listdir):
        # Mock os.listdir and FileStat
        mock_listdir.return_value = ["file1.npy", "file2.bin", "file3.txt"]
        mock_file_stat.return_value.is_basically_legal.return_value = True
        mock_file_stat.return_value.is_dir = False

        # Test getting files list from directory
        dir_ = "/path/to/directory"
        result = get_fileslist_from_dir(dir_)
        expected = ["/path/to/directory/file1.npy", "/path/to/directory/file2.bin"]
        self.assertEqual(result, expected)

    def test_get_file_datasize(self):
        # Test getting file data size
        file_path = "test_file.npy"
        np.save(file_path, np.array([1, 2, 3]))
        result = get_file_datasize(file_path)
        os.path.getsize(file_path)
        self.assertEqual(result, 24)
        os.remove(file_path)

    def test_get_file_content(self):
        # Test getting file content
        file_path = "test_file.npy"
        np.save(file_path, np.array([1, 2, 3]))
        result = get_file_content(file_path)
        expected = np.array([1, 2, 3])
        np.testing.assert_array_equal(result, expected)
        os.remove(file_path)

    @patch("ais_bench.infer.common.utils.ms_open")
    def test_get_file_content1(self, mock_ms_open):
        bin_data = bytes([1, 2, 3, 4, 5])
        expected_array = np.frombuffer(bin_data, dtype=np.int8)

        mock_ms_open.return_value = MockMsOpen(bin_data)
        file_path = "test.bin"

        result = get_file_content(file_path)

        assert np.array_equal(result, expected_array)

    def test_get_ndata_fmt(self):
        # Test getting data format
        ndata = np.array([1, 2, 3], dtype=np.float32)
        result = get_ndata_fmt(ndata)
        expected = "%f"
        self.assertEqual(result, expected)

        ndata = np.array([1, 2, 3], dtype=np.int32)
        result = get_ndata_fmt(ndata)
        expected = "%d"
        self.assertEqual(result, expected)

    def test_save_data_to_files(self):
        # Test saving data to files
        file_path = "test_file.npy"
        ndata = np.array([1, 2, 3])
        save_data_to_files(file_path, ndata)
        loaded_data = np.load(file_path)
        np.testing.assert_array_equal(loaded_data, ndata)
        os.remove(file_path)

    def test_save_data_to_files1(self):
        # Test saving data to files
        file_path = "test_file.txt"
        ndata = np.array([1, 2, 3])
        save_data_to_files(file_path, ndata)
        loaded_data = np.loadtxt(file_path)
        np.testing.assert_array_equal(loaded_data, ndata)
        os.remove(file_path)

    def test_create_fake_file_name(self):
        # Test creating fake file name
        pure_data_type = "random"
        index = 1
        result = create_fake_file_name(pure_data_type, index)
        self.assertEqual(result[-8:], "random_1")

    def test_get_dump_relative_paths(self):
        # Test getting dump relative paths
        output_dir = "/path/to/output"
        timestamp = "20230101"
        result = get_dump_relative_paths(output_dir, timestamp)
        self.assertIsInstance(result, list)

    def test_get_msprof_bin_path(self):
        # Test getting msprof bin path
        result = get_msprof_bin_path()
        self.assertIsInstance(result, str)

    def test_get_msaccucmp_path(self):
        # Test getting msaccucmp path
        result = get_msaccucmp_path()
        self.assertIsInstance(result, str)

    def test_make_dirs(self):
        # Test making directories
        path = "test_dir"
        result = make_dirs(path)
        self.assertEqual(result, 0)
        self.assertTrue(os.path.exists(path))
        os.rmdir(path)

    @patch("ais_bench.infer.common.utils.ms_open")
    @patch("json.load")
    @patch("os.remove")
    def test_create_tmp_acl_json(self, mock_os_remove, mock_json_load, mock_ms_open):
        # Mock ms_open and json.load
        mock_os_remove.return_value = None
        mock_ms_open.return_value.__enter__.return_value.read.return_value = (
            '{"dump": {"dump_path": "dump"}}'
        )
        mock_json_load.return_value = {"dump": {"dump_path": "dump"}}

        # Test creating temporary ACL JSON
        acl_json_path = "acl.json"
        result = create_tmp_acl_json(acl_json_path)
        self.assertIsInstance(result, tuple)

    @patch("ais_bench.infer.common.utils.get_dump_relative_paths")
    @patch("ais_bench.infer.common.utils.get_msaccucmp_path")
    @patch("ais_bench.infer.common.utils.subprocess.call")
    def test_convert_helper(
        self,
        mock_subprocess_call,
        mock_get_msaccucmp_path,
        mock_get_dump_relative_paths,
    ):
        # Mock get_dump_relative_paths, get_msaccucmp_path, and subprocess.call
        mock_get_dump_relative_paths.return_value = ["/path/to/dump"]
        mock_get_msaccucmp_path.return_value = "/path/to/msaccucmp.py"
        mock_subprocess_call.return_value = 0

        # Test converting helper
        output_dir = "/path/to/output"
        timestamp = "20230101"
        convert_helper(output_dir, timestamp)

    @patch("ais_bench.infer.common.utils.get_dump_relative_paths")
    @patch("ais_bench.infer.common.utils.get_msaccucmp_path")
    @patch("ais_bench.infer.common.utils.subprocess.call")
    def test_convert_helper1(
        self,
        mock_subprocess_call,
        mock_get_msaccucmp_path,
        mock_get_dump_relative_paths,
    ):
        # Mock get_dump_relative_paths, get_msaccucmp_path, and subprocess.call
        mock_get_dump_relative_paths.return_value = ["/path/to/dump"]
        mock_get_msaccucmp_path.return_value = None
        mock_subprocess_call.return_value = 0

        # Test converting helper
        output_dir = "/path/to/output"
        timestamp = "20230101"
        convert_helper(output_dir, timestamp)

    @patch("ais_bench.infer.common.utils.get_dump_relative_paths")
    @patch("ais_bench.infer.common.utils.get_msaccucmp_path")
    @patch("ais_bench.infer.common.utils.subprocess.call")
    def test_convert_helper2(
        self,
        mock_subprocess_call,
        mock_get_msaccucmp_path,
        mock_get_dump_relative_paths,
    ):
        # Mock get_dump_relative_paths, get_msaccucmp_path, and subprocess.call
        mock_get_dump_relative_paths.return_value = None
        mock_get_msaccucmp_path.return_value = 11
        mock_subprocess_call.return_value = 0

        # Test converting helper
        output_dir = "/path/to/output"
        timestamp = "20230101"
        convert_helper(output_dir, timestamp)

    @patch("ais_bench.infer.common.utils.check_path_legality")
    @patch("shutil.move")
    @patch("os.listdir")
    def test_move_subdir(self, mock_listdir, mock_move, mock_check_path_legality):
        # Mock os.listdir and shutil.move
        mock_listdir.return_value = ["subdir"]
        mock_move.return_value = None
        mock_check_path_legality.return_value = None

        # Test moving subdirectory
        src_dir = "/path/to/src"
        dest_dir = "/path/to/dest"
        result = move_subdir(src_dir, dest_dir)
        self.assertEqual(result, (dest_dir, "subdir"))

    @patch("ais_bench.infer.common.utils.check_path_legality")
    @patch("os.listdir")
    def test_move_subdir1(self, mock_listdir, mock_check_path_legality):
        # Mock os.listdir and shutil.move
        mock_listdir.return_value = ["subdir"]
        mock_check_path_legality.return_value = None

        # Test moving subdirectory
        src_dir = "/path/to/src"
        dest_dir = "/path/to/dest"
        with self.assertRaises(RuntimeError):
            move_subdir(src_dir, dest_dir)

    def test_str_to_uint(self):
        # Test converting string to uint
        string = "123"
        result = str_to_uint(string)
        expected = 123
        self.assertEqual(result, expected)

    def test_str_to_uint1(self):
        # Test converting string to uint
        string = 123
        with self.assertRaises(ValueError):
            str_to_uint(string)

    def test_str_to_uint2(self):
        # Test converting string to uint
        string = ""
        with self.assertRaises(ValueError):
            str_to_uint(string)

    def test_str_to_uint3(self):
        # Test converting string to uint
        string = "xdfa"
        with self.assertRaises(ValueError):
            str_to_uint(string)


if __name__ == "__main__":
    unittest.main()
