import unittest
from unittest.mock import MagicMock, patch
import numpy as np
from ais_bench.infer.common.io_operations import (
    PURE_INFER_FAKE_FILE,
    PURE_INFER_FAKE_FILE_ZERO,
    PADDING_INFER_FAKE_FILE,
    convert_real_files,
    get_pure_infer_data,
    get_narray_from_files_list,
    get_files_count_per_batch,
    create_infileslist_from_fileslist,
    get_tensor_from_files_list,
    create_intensors_from_infileslist,
    check_input_parameter,
    create_infileslist_from_inputs_list,
    check_pipeline_fileslist_match_intensors,
    create_pipeline_fileslist_from_inputs_list,
    save_tensors_to_file,
)


class TestConvertRealFiles(unittest.TestCase):
    def test_normal_files(self):
        files = ["file1.txt", "file2.txt", "file3.txt"]
        expected = ["file1.txt", "file2.txt", "file3.txt"]
        self.assertEqual(convert_real_files(files), expected)

    def test_pure_infer_fake_file(self):
        files = ["file1.txt", PURE_INFER_FAKE_FILE, "file3.txt"]
        with self.assertRaises(RuntimeError) as context:
            convert_real_files(files)
        self.assertIn("not support pure infer", str(context.exception))

    def test_npy_file(self):
        files = ["file1.txt", "file2.npy", "file3.NPY"]
        with self.assertRaises(RuntimeError) as context:
            convert_real_files(files)
        self.assertIn("not support npy file:file2.npy", str(context.exception))


class TestGetPureInferData(unittest.TestCase):
    def test_random_data(self):
        size = 10
        data = get_pure_infer_data(size, "random")
        self.assertIsInstance(data, np.ndarray)  # 检查返回类型
        self.assertEqual(data.shape, (size,))  # 检查数据长度
        self.assertTrue(np.all(data >= 0) and np.all(data <= 255))  # 检查数据范围

    def test_zero_data(self):
        size = 10
        data = get_pure_infer_data(size, "zero")
        self.assertIsInstance(data, np.ndarray)  # 检查返回类型
        self.assertEqual(data.shape, (size,))  # 检查数据长度
        self.assertTrue(np.all(data == 0))  # 检查数据是否全零

    def test_default_data(self):
        size = 10
        data = get_pure_infer_data(size, "other")  # 使用非 "random" 的类型
        self.assertIsInstance(data, np.ndarray)  # 检查返回类型
        self.assertEqual(data.shape, (size,))  # 检查数据长度
        self.assertTrue(np.all(data == 0))  # 检查数据是否全零


class TestGetNarrayFromFileList(unittest.TestCase):
    @patch("os.path.exists")
    @patch("numpy.concatenate")
    @patch("ais_bench.infer.common.io_operations.get_file_content")
    def test_get_narray_from_files_list(
        self, mock_file_content, mock_concatenate, mock_exists
    ):
        # 模拟 os.path.exists 的行为
        mock_exists.return_value = True

        # 模拟 np.concatenate 的行为
        mock_concatenate.return_value = np.array([1, 2, 3, 4, 5])

        mock_file_content.return_value = "2"

        # 测试文件路径在 file_path_switch 中
        files_list = [PURE_INFER_FAKE_FILE, PURE_INFER_FAKE_FILE_ZERO]
        size = 40
        pure_data_type = "zero"
        result = get_narray_from_files_list(files_list, size, pure_data_type)
        self.assertTrue(np.array_equal(result, np.array([1, 2, 3, 4, 5])))

        # 测试文件路径为 PADDING_INFER_FAKE_FILE
        files_list = [PADDING_INFER_FAKE_FILE, PURE_INFER_FAKE_FILE]
        result = get_narray_from_files_list(files_list, size, pure_data_type)

        self.assertTrue(np.array_equal(result, np.array([1, 2, 3, 4, 5])))

        # 测试文件路径不存在
        files_list = [None]
        with self.assertRaises(RuntimeError):
            get_narray_from_files_list(files_list, size, pure_data_type)

        # 测试文件路径有效和len(ndatalist) == 1
        files_list = ["valid_file"]
        result = get_narray_from_files_list(files_list, size, pure_data_type)
        self.assertTrue(np.array_equal(result, "2"))

        # 测试 len(ndatalist) > 1
        files_list = ["valid_file", "valid_file"]
        result = get_narray_from_files_list(files_list, size, pure_data_type)
        print(f"{result=}")
        self.assertTrue(np.array_equal(result, np.array([1, 2, 3, 4, 5])))

        files_list = ["valid_file", "valid_file"]
        with self.assertRaises(RuntimeError):
            result = get_narray_from_files_list(files_list, 4, pure_data_type)


class TestGetTensorFromFileList(unittest.TestCase):
    @patch("ais_bench.infer.common.io_operations.get_narray_from_files_list")
    def test_get_tensor_from_files_list_given_valid_files_when_no_combine_tensor_mode_then_success(
        self, mock_get_narray
    ):
        # 模拟 session.create_tensor_from_arrays_to_device
        session = MagicMock()
        session.create_tensor_from_arrays_to_device.return_value = "tensor"

        # 测试参数
        files_list = ["file1", "file2"]
        size = 10
        pure_data_type = "zero"
        no_combine_tensor_mode = True

        # 调用函数
        result = get_tensor_from_files_list(
            files_list, session, size, pure_data_type, no_combine_tensor_mode
        )

        # 验证结果
        self.assertEqual(result, "tensor")
        mock_get_narray.assert_called_once_with(
            files_list, size, pure_data_type, no_combine_tensor_mode
        )
        session.create_tensor_from_arrays_to_device.assert_called_once()


class TensorDesc:
    def __init__(self, realsize):
        self.realsize = realsize


class TestGetFilesCountPerBatch(unittest.TestCase):
    @patch("math.ceil")
    @patch("os.path.getsize")
    def test_get_files_count_per_batch_given_valid_input_when_no_combine_tensor_mode_then_returns_1_and_runcount(
        self, mock_datasize, mock_ceil
    ):
        # Arrange
        mock_ceil.return_value = 5
        mock_datasize.return_value = 10
        intensors_desc = [TensorDesc(realsize=100)]
        fileslist = [["file1", "file2", "file3", "file4", "file5"]]
        no_combine_tensor_mode = True

        # Act
        files_count_per_batch, runcount = get_files_count_per_batch(
            intensors_desc, fileslist, no_combine_tensor_mode
        )

        # Assert
        self.assertEqual(files_count_per_batch, 1)
        self.assertEqual(runcount, 5)

    @patch("math.ceil")
    @patch("os.path.getsize")
    def test_get_files_count_per_batch_given_valid_input_when_combine_tensor_mode_then_returns_correct_values(
        self, mock_datasize, mock_ceil
    ):
        # Arrange
        mock_ceil.return_value = 2
        mock_datasize.return_value = 10
        intensors_desc = [TensorDesc(realsize=100)]
        fileslist = [["file1", "file2", "file3", "file4", "file5"]]
        no_combine_tensor_mode = False

        # Act
        files_count_per_batch, runcount = get_files_count_per_batch(
            intensors_desc, fileslist, no_combine_tensor_mode
        )

        # Assert
        self.assertEqual(files_count_per_batch, 10)
        self.assertEqual(runcount, 2)

    @patch("os.path.getsize")
    def test_get_files_count_per_batch_given_invalid_filesize_when_combine_tensor_mode_then_raises_runtime_error(
        self, mock_datasize
    ):
        # Arrange
        mock_datasize.return_value = 3
        intensors_desc = [TensorDesc(realsize=10)]
        fileslist = [["file1", "file2", "file3", "file4", "file5"]]
        no_combine_tensor_mode = False

        # Act and Assert
        with self.assertRaises(RuntimeError):
            get_files_count_per_batch(intensors_desc, fileslist, no_combine_tensor_mode)

    @patch("os.path.getsize")
    def test_get_files_count_per_batch_given_invalid_tensorsize_when_combine_tensor_mode_then_raises_runtime_error(
        self, mock_datasize
    ):
        # Arrange
        mock_datasize.return_value = 101
        intensors_desc = [TensorDesc(realsize=0)]
        fileslist = [["file1", "file2", "file3", "file4", "file5"]]
        no_combine_tensor_mode = False

        # Act and Assert
        with self.assertRaises(RuntimeError):
            get_files_count_per_batch(intensors_desc, fileslist, no_combine_tensor_mode)


class TestCreateInfileslistFromFileslist(unittest.TestCase):
    @patch("ais_bench.infer.common.io_operations.get_files_count_per_batch")
    @patch("ais_bench.infer.common.io_operations.list_split")
    def test_create_infileslist_from_fileslist_given_matching_lengths_when_no_combine_tensor_mode_then_success(
        self, mock_list_split, mock_get_files_count_per_batch
    ):
        mock_get_files_count_per_batch.return_value = (2, 2)
        mock_list_split.return_value = [1, 2]

        fileslist = ["file1", "file2", "file3", "file4"]
        intensors_desc = [1, 2, 3, 4]
        no_combine_tensor_mode = False

        # Act
        result = create_infileslist_from_fileslist(
            fileslist, intensors_desc, no_combine_tensor_mode
        )
        print(f"{result=}")
        # Assert
        self.assertEqual(len(result), 2)
        self.assertEqual(len(result[0]), 4)
        self.assertEqual(len(result[1]), 4)
        self.assertEqual(result[0], [1, 1, 1, 1])
        self.assertEqual(result[1], [2, 2, 2, 2])

    @patch("ais_bench.infer.common.io_operations.get_files_count_per_batch")
    def test_create_infileslist_from_fileslist_given_mismatching_lengths_when_no_combine_tensor_mode_then_failure(
        self, mock_get_files_count_per_batch
    ):
        # Arrange
        fileslist = ["file1", "file2", "file3"]
        intensors_desc = [1, 2]
        no_combine_tensor_mode = False
        mock_get_files_count_per_batch.return_value = (2, 2)

        # Act and Assert
        with self.assertRaises(RuntimeError):
            create_infileslist_from_fileslist(
                fileslist, intensors_desc, no_combine_tensor_mode
            )

    @patch("ais_bench.infer.common.io_operations.get_files_count_per_batch")
    def test_create_infileslist_from_fileslist_given_mismatching_lengths_when_combine_tensor_mode_then_failure(
        self, mock_get_files_count_per_batch
    ):
        # Arrange
        fileslist = ["file1", "file2", "file3"]
        intensors_desc = [1, 2]
        no_combine_tensor_mode = True
        mock_get_files_count_per_batch.return_value = (2, 2)

        # Act and Assert
        with self.assertRaises(RuntimeError):
            create_infileslist_from_fileslist(
                fileslist, intensors_desc, no_combine_tensor_mode
            )


class TestCreateIntensorsFromInfileslist(unittest.TestCase):
    @patch("ais_bench.infer.common.io_operations.get_tensor_from_files_list")
    def test_create_intensors_from_infileslist_given_valid_infileslist_then_success(
        self, mock_get_tensor_from_files_list
    ):
        # Arrange
        infileslist = [["file1", "file2"], ["file3", "file4"]]
        intensors_desc = [TensorDesc(realsize=10), TensorDesc(realsize=20)]
        session = MagicMock()
        pure_data_type = "float32"
        no_combine_tensor_mode = False
        mock_get_tensor_from_files_list.return_value = 3
        # Act
        result = create_intensors_from_infileslist(
            infileslist, intensors_desc, session, pure_data_type, no_combine_tensor_mode
        )

        # Assert
        self.assertEqual(len(result), 2)
        self.assertEqual(len(result[0]), 2)
        self.assertEqual(len(result[1]), 2)
        self.assertEqual(result[0][0], 3)
        self.assertEqual(result[1][0], 3)


class TestCheckInputParameter(unittest.TestCase):
    @patch("os.path.isfile")
    @patch("os.path.isdir")
    @patch("os.readlink")
    def test_check_input_parameter_given_valid_files_then_success(
        self, mock_readlink, mock_isdir, mock_isfile
    ):
        # Arrange
        inputs_list = ["file1", "file2"]
        intensors_desc = [TensorDesc(realsize=10), TensorDesc(realsize=20)]
        mock_isfile.return_value = True
        mock_readlink.return_value = "real_file1"

        # Act
        check_input_parameter(inputs_list, intensors_desc)

    @patch("os.path.isfile")
    @patch("os.path.isdir")
    @patch("os.readlink")
    def test_check_input_parameter_given_valid_dirs_then_success(
        self, mock_readlink, mock_isdir, mock_isfile
    ):
        # Arrange
        inputs_list = ["dir1", "dir2"]
        intensors_desc = [TensorDesc(realsize=10), TensorDesc(realsize=20)]
        mock_isdir.return_value = True
        mock_readlink.return_value = "real_dir1"

        # Act
        check_input_parameter(inputs_list, intensors_desc)

    @patch("os.path.isfile")
    @patch("os.path.isdir")
    @patch("os.readlink")
    def test_check_input_parameter_given_invalid_files_then_failure(
        self, mock_readlink, mock_isdir, mock_isfile
    ):
        # Arrange
        inputs_list = ["file1", "file2"]
        intensors_desc = [TensorDesc(realsize=10)]
        mock_isfile.return_value = False
        mock_readlink.return_value = "real_file1"

        # Act and Assert
        with self.assertRaises(RuntimeError):
            check_input_parameter(inputs_list, intensors_desc)

    @patch("os.path.isfile")
    @patch("os.path.isdir")
    @patch("os.readlink")
    def test_check_input_parameter_given_invalid_dirs_then_failure(
        self, mock_readlink, mock_isdir, mock_isfile
    ):
        # Arrange
        inputs_list = ["dir1", "dir2"]
        intensors_desc = [TensorDesc(realsize=10), TensorDesc(realsize=20)]
        mock_isdir.return_value = False
        mock_isfile.return_value = False
        mock_readlink.return_value = "real_dir1"

        # Act and Assert
        with self.assertRaises(RuntimeError):
            check_input_parameter(inputs_list, intensors_desc)

    @patch("os.path.isfile")
    @patch("os.path.isdir")
    @patch("os.readlink")
    def test_check_input_parameter_given_valid_dirs_then_true(
        self, mock_readlink, mock_isdir, mock_isfile
    ):
        # Arrange
        inputs_list = ["dir1", "dir2"]
        intensors_desc = [TensorDesc(realsize=10), TensorDesc(realsize=20)]
        mock_isdir.return_value = True
        mock_isfile.return_value = False
        mock_readlink.return_value = False
        mock_readlink.return_value = "real_dir1"

        check_input_parameter(inputs_list, intensors_desc)


class TestCreateInfileslistFromInputsList(unittest.TestCase):
    @patch("ais_bench.infer.common.io_operations.create_infileslist_from_fileslist")
    @patch("os.path.isfile")
    def test_create_infileslist_from_inputs_list_given_files_when_success_then_returns_infileslist(
        self, mock_isfile, mock_create_infileslist_from_fileslist
    ):
        # Arrange
        mock_isfile.return_value = True
        inputs_list = ["file1.txt", "file2.txt", "file3.txt"]
        intensors_desc = [1, 2, 3]
        no_combine_tensor_mode = False
        mock_create_infileslist_from_fileslist.return_value = [
            ["file1", "file2"],
            ["file3", "file4"],
        ]

        # Act
        result = create_infileslist_from_inputs_list(
            inputs_list, intensors_desc, no_combine_tensor_mode
        )

        # Assert
        self.assertEqual(result, [["file1", "file2"], ["file3", "file4"]])

    @patch("ais_bench.infer.common.io_operations.get_fileslist_from_dir")
    @patch("ais_bench.infer.common.io_operations.create_infileslist_from_fileslist")
    @patch("os.path.isfile")
    @patch("os.path.isdir")
    def test_create_infileslist_from_inputs_list_given_dirs_when_success_then_returns_infileslist(
        self,
        mock_isdir,
        mock_isfile,
        mock_create_infileslist_from_fileslist,
        mock_get_fileslist_from_dir,
    ):
        # Arrange
        mock_isfile.return_value = False
        mock_isdir.return_value = True
        inputs_list = ["file1.txt", "file2.txt", "file3.txt"]
        intensors_desc = [1, 2, 3]
        no_combine_tensor_mode = False
        mock_create_infileslist_from_fileslist.return_value = [
            ["file1", "file2"],
            ["file3", "file4"],
        ]
        mock_get_fileslist_from_dir.return_value = 1

        # Act
        result = create_infileslist_from_inputs_list(
            inputs_list, intensors_desc, no_combine_tensor_mode
        )

        # Assert
        self.assertEqual(result, [["file1", "file2"], ["file3", "file4"]])

    @patch("os.path.isfile")
    @patch("os.path.isdir")
    def test_create_infileslist_from_inputs_list_given_none_when_success_then_returns_infileslist(
        self, mock_isdir, mock_isfile
    ):
        # Arrange
        mock_isfile.return_value = False
        mock_isdir.return_value = False
        inputs_list = ["file1.txt", "file2.txt", "file3.txt"]
        intensors_desc = [1, 2, 3]
        no_combine_tensor_mode = False

        # Act
        with self.assertRaises(RuntimeError):
            create_infileslist_from_inputs_list(
                inputs_list, intensors_desc, no_combine_tensor_mode
            )

    @patch("ais_bench.infer.common.io_operations.get_fileslist_from_dir")
    @patch("ais_bench.infer.common.io_operations.create_infileslist_from_fileslist")
    @patch("os.path.isfile")
    @patch("os.path.isdir")
    def test_create_infileslist_from_inputs_list_given_dirs_when_create_result_is_none(
        self,
        mock_isdir,
        mock_isfile,
        mock_create_infileslist_from_fileslist,
        mock_get_fileslist_from_dir,
    ):
        # Arrange
        mock_isfile.return_value = False
        mock_isdir.return_value = True
        inputs_list = ["file1.txt", "file2.txt", "file3.txt"]
        intensors_desc = [1, 2, 3]
        no_combine_tensor_mode = False
        mock_create_infileslist_from_fileslist.return_value = []
        mock_get_fileslist_from_dir.return_value = 1

        # Act
        with self.assertRaises(RuntimeError):
            create_infileslist_from_inputs_list(
                inputs_list, intensors_desc, no_combine_tensor_mode
            )


class TestCheckPipelineFileslistMatchIntensors(unittest.TestCase):
    @patch("ais_bench.infer.common.io_operations.get_file_datasize")
    def test_check_pipeline_fileslist_match_intensors_given_matching_lengths_and_sizes_then_success(
        self, mock_get_file_datasize
    ):
        # Arrange
        fileslist = [["file1"], ["file2"], ["file3"]]
        intensors_desc = [
            MagicMock(
                realsize=1024,
                size=1024,
                shape=[10, 10],
                auto_dim_mode=False,
                auto_shape_mode=False,
            ),
            MagicMock(
                realsize=1024,
                size=1024,
                shape=[10, 10],
                auto_dim_mode=False,
                auto_shape_mode=False,
            ),
            MagicMock(
                realsize=1024,
                size=1024,
                shape=[10, 10],
                auto_dim_mode=False,
                auto_shape_mode=False,
            ),
        ]
        mock_get_file_datasize.return_value = 1024

        # Act
        check_pipeline_fileslist_match_intensors(fileslist, intensors_desc)

        # Assert
        self.assertTrue(True)  # No exception should be raised

    @patch("ais_bench.infer.common.io_operations.get_file_datasize")
    def test_check_pipeline_fileslist_match_intensors_given_mismatching_lengths_then_failure(
        self, mock_get_file_datasize
    ):
        # Arrange
        fileslist = [["file1"], ["file2"], ["file3"]]
        intensors_desc = [
            MagicMock(
                realsize=1024,
                size=1324,
                shape=[10, 10],
                auto_dim_mode=False,
                auto_shape_mode=False,
            ),
            MagicMock(
                realsize=1024,
                size=1324,
                shape=[10, 10],
                auto_dim_mode=False,
                auto_shape_mode=False,
            ),
        ]
        mock_get_file_datasize.return_value = 2048

        # Act and Assert
        with self.assertRaises(RuntimeError):
            check_pipeline_fileslist_match_intensors(fileslist, intensors_desc)


class TestCreatePipelineFileslistFromInputsList(unittest.TestCase):
    @patch("ais_bench.infer.common.io_operations.list_split")
    @patch("ais_bench.infer.common.io_operations.check_input_parameter")
    @patch(
        "ais_bench.infer.common.io_operations.check_pipeline_fileslist_match_intensors"
    )
    @patch("os.path.isfile")
    def test_create_pipeline_fileslist_from_inputs_list_given_valid_file_inputs_then_success(
        self,
        mock_isfile,
        mock_check_pipeline_fileslist_match_intensors,
        mock_check_input_parameter,
        mock_list_split,
    ):
        # Arrange
        mock_isfile.return_value = True
        inputs_list = ["file1", "file2", "file3", "file4"]
        intensors_desc = [1, 2]
        mock_check_input_parameter.return_value = None
        mock_list_split.return_value = [["file1", "file2"], ["file3", "file4"]]
        mock_check_pipeline_fileslist_match_intensors.return_value = None

        # Act
        result = create_pipeline_fileslist_from_inputs_list(inputs_list, intensors_desc)

        # Assert
        self.assertEqual(result, [("file1", "file3"), ("file2", "file4")])
        mock_check_input_parameter.assert_called_once_with(inputs_list, intensors_desc)
        mock_list_split.assert_called_once_with(inputs_list, 2, PADDING_INFER_FAKE_FILE)
        mock_check_pipeline_fileslist_match_intensors.assert_called_once_with(
            mock_list_split.return_value, intensors_desc
        )

    @patch("ais_bench.infer.common.io_operations.get_fileslist_from_dir")
    @patch("ais_bench.infer.common.io_operations.check_input_parameter")
    @patch(
        "ais_bench.infer.common.io_operations.check_pipeline_fileslist_match_intensors"
    )
    @patch("os.path.isdir")
    def test_create_pipeline_fileslist_from_inputs_list_given_valid_dir_inputs_then_success(
        self,
        mock_isdir,
        mock_check_pipeline_fileslist_match_intensors,
        mock_check_input_parameter,
        mock_get_fileslist_from_dir,
    ):
        # Arrange
        mock_isdir.return_value = True
        inputs_list = ["file1", "file2", "file3", "file4"]
        intensors_desc = [1, 2, 3, 4]
        mock_check_input_parameter.return_value = None
        mock_get_fileslist_from_dir.return_value = ["file1", "file2", "file3", "file4"]
        mock_check_pipeline_fileslist_match_intensors.return_value = None

        # Act
        result = create_pipeline_fileslist_from_inputs_list(inputs_list, intensors_desc)

        # Assert
        self.assertEqual(result[0], ("file1", "file1", "file1", "file1"))
        mock_check_input_parameter.assert_called_once_with(inputs_list, intensors_desc)

    @patch("ais_bench.infer.common.io_operations.check_input_parameter")
    @patch("os.path.isdir")
    @patch("os.path.isfile")
    def test_create_pipeline_fileslist_from_inputs_list_given_invalid_dir_inputs_then_failure(
        self, mock_isfile, mock_isdir, mock_check_input_parameter
    ):
        # Arrange
        mock_isfile.return_value = False
        mock_isdir.return_value = False
        inputs_list = ["file1", "file2", "file3", "file4"]
        intensors_desc = [1, 2, 3, 4]
        mock_check_input_parameter.return_value = None

        # Act
        with self.assertRaises(RuntimeError):
            create_pipeline_fileslist_from_inputs_list(inputs_list, intensors_desc)

    @patch("ais_bench.infer.common.io_operations.get_fileslist_from_dir")
    @patch("ais_bench.infer.common.io_operations.check_input_parameter")
    @patch("os.path.isdir")
    def test_create_pipeline_fileslist_from_inputs_list_given_valid_dir_inputs_then_failure(
        self, mock_isdir, mock_check_input_parameter, mock_get_fileslist_from_dir
    ):
        # Arrange
        mock_isdir.return_value = True
        inputs_list = ["file1", "file2", "file3", "file4"]
        intensors_desc = [1, 2, 3, 4]
        mock_check_input_parameter.return_value = None
        mock_get_fileslist_from_dir.return_value = ["file1", "file2", "file3", "file4"]

        # Act
        with self.assertRaises(RuntimeError):
            create_pipeline_fileslist_from_inputs_list(inputs_list, intensors_desc)


class OutFileInfo:
    def __init__(self, output_prefix, outputs):
        self.output_prefix = output_prefix
        self.outputs = outputs


class TestSaveTensorsToFile(unittest.TestCase):
    @patch("os.path.join")
    @patch("os.path.basename")
    @patch("numpy.transpose")
    @patch("numpy.array")
    @patch("numpy.array_split")
    def test_save_tensors_to_file_given_invalid_inputs_then_failure(
        self, mock_array_split, mock_array, mock_transpose, mock_basename, mock_join
    ):
        # Arrange
        infiles_paths = [["file1.txt", "file2.txt"], ["file3.txt", "file4.txt"]]
        outfmt = "npy"
        index = 0
        output_batchsize_axis = 0
        out_file_info = OutFileInfo(
            output_prefix="/output", outputs=[np.array([1, 2, 3]), np.array([4, 5, 6])]
        )
        mock_transpose.return_value = [
            ["file1.txt", "file3.txt"],
            ["file2.txt", "file4.txt"],
        ]
        mock_array.side_effect = [np.array([1, 2, 3]), np.array([4, 5, 6])]
        mock_array_split.return_value = -1
        mock_basename.return_value = "file1.txt"
        mock_join.return_value = "/output/file1_0.npy"

        # Act
        with self.assertRaises(RuntimeError):
            save_tensors_to_file(
                infiles_paths, outfmt, index, output_batchsize_axis, out_file_info
            )

    @patch("os.path.join")
    @patch("os.path.basename")
    @patch("numpy.transpose")
    @patch("numpy.array")
    @patch("numpy.array_split")
    def test_save_tensors_to_file_given_invalid_inputs_then_failure1(
        self, mock_array_split, mock_array, mock_transpose, mock_basename, mock_join
    ):
        # Arrange
        infiles_paths = [["file1.txt", "file2.txt"], ["file3.txt", "file4.txt"]]
        outfmt = "npy"
        index = 0
        output_batchsize_axis = -1
        out_file_info = OutFileInfo(
            output_prefix="/output", outputs=[np.array([1, 2, 3]), np.array([4, 5, 6])]
        )
        mock_transpose.return_value = [
            ["file1.txt", "file3.txt"],
            ["file2.txt", "file4.txt"],
        ]
        mock_array.side_effect = [np.array([1, 2, 3]), np.array([4, 5, 6])]
        mock_array_split.return_value = 0
        mock_basename.return_value = "file1.txt"
        mock_join.return_value = "/output/file1_0.npy"

        # Act
        with self.assertRaises(RuntimeError):
            save_tensors_to_file(
                infiles_paths, outfmt, index, output_batchsize_axis, out_file_info
            )

    @patch("os.path.join")
    @patch("os.path.basename")
    @patch("numpy.transpose")
    @patch("ais_bench.infer.common.io_operations.np.array")
    @patch("ais_bench.infer.common.io_operations.np.array_split")
    @patch("ais_bench.infer.common.io_operations.save_data_to_files")
    def test_save_tensors_to_file_given_valid_inputs_then_success(
        self,
        mock_save_data,
        mock_array_split,
        mock_array,
        mock_transpose,
        mock_basename,
        mock_join,
    ):
        # Arrange
        infiles_paths = [["file1.txt"], ["file3.txt"]]
        outfmt = "npy"
        index = 0
        output_batchsize_axis = -1
        out_file_info = OutFileInfo(
            output_prefix="/output", outputs=[np.array([1, 2, 3]), np.array([4, 5, 6])]
        )
        mock_transpose.return_value = [
            ["file1.txt", "file3.txt"],
            ["file2.txt", "file4.txt"],
        ]
        mock_array.return_value = MagicMock(shape=[2, 1, 3])
        mock_array_split.return_value = [1]
        mock_basename.return_value = "file1.txt"
        mock_join.return_value = "/output/file1_0.npy"
        mock_save_data.return_value = None

        # Act
        save_tensors_to_file(
            infiles_paths, outfmt, index, output_batchsize_axis, out_file_info
        )


if __name__ == "__main__":
    unittest.main()
