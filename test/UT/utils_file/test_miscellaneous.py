import unittest
from unittest.mock import MagicMock, patch
from ais_bench.infer.common.miscellaneous import (
    get_modules_version,
    version_check,
    get_model_name,
    check_dump_path,
    check_valid_acl_json_for_dump,
    get_acl_json_path,
    get_batchsize,
    get_range_list,
    get_dymshape_list,
    get_throughtput_from_log,
    regenerate_dymshape_cmd,
    dymshape_range_run,
    AISBenchInferArgsAdapter,
)


class TestYourModule(unittest.TestCase):
    @patch("pkg_resources.get_distribution")
    def test_get_modules_version(self, mock_get_distribution):
        mock_get_distribution.return_value.version = "1.0.0"
        version = get_modules_version("aclruntime")
        self.assertEqual(version, "1.0.0")

    @patch("pkg_resources.get_distribution")
    def test_version_check(self, mock_get_distribution):
        mock_get_distribution.return_value.version = "0.0.3"
        args = MagicMock()
        args.run_mode = None
        version_check(args)
        self.assertEqual(args.run_mode, "tensor")

    def test_get_model_name(self):
        model_name = get_model_name("/path/to/model.om")
        self.assertEqual(model_name, "model")

    @patch("ais_bench.infer.common.miscellaneous.FileStat")
    def test_check_dump_path(self, mock_file_stat):
        mock_file_stat.return_value.is_dir = False
        mock_file_stat.return_value.is_exists = True
        mock_file_stat.return_value.is_basically_legal.return_value = True
        with self.assertRaises(TypeError):
            check_dump_path("/path/to/dump")

    @patch("ais_bench.infer.common.miscellaneous.FileStat")
    def test_check_dump_path1(self, mock_file_stat):
        mock_file_stat.return_value.is_dir = True
        mock_file_stat.return_value.is_exists = True
        mock_file_stat.return_value.is_basically_legal.return_value = False
        with self.assertRaises(ValueError):
            check_dump_path("/path/to/dump")

    def test_check_dump_path2(self):
        with self.assertRaises(ValueError):
            check_dump_path(33)

    @patch("ais_bench.infer.common.miscellaneous.check_dump_path")
    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_check_valid_acl_json_for_dump(self, mock_ms_open, mock_check_dump_path):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = '{"dump": {"dump_list": [{"model_name": "model"}], "dump_path": "/path/to/dump"}}'
        mock_check_dump_path.return_value = None
        check_valid_acl_json_for_dump("/path/to/acl.json", "/path/to/model.om")

    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_check_valid_acl_json_for_dump_fail1(self, mock_ms_open):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = '{"dump": {"dump_lists": [{"model_name": "model"}], "dump_path": "/path/to/dump"}}'
        with self.assertRaises(KeyError):
            check_valid_acl_json_for_dump("/path/to/acl.json", "/path/to/model.om")

    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_check_valid_acl_json_for_dump_fail2(self, mock_ms_open):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = (
            '{"dump": {"dump_list": [], "dump_path": "/path/to/dump"}}'
        )
        with self.assertRaises(ValueError):
            check_valid_acl_json_for_dump("/path/to/acl.json", "/path/to/model.om")

    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_check_valid_acl_json_for_dump_fail3(self, mock_ms_open):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = '{"dump": {"dump_list": [{"model_name": "model"}], "dump_paths": "/path/to/dump"}}'
        with self.assertRaises(KeyError):
            check_valid_acl_json_for_dump("/path/to/acl.json", "/path/to/model.om")

    @patch("ais_bench.infer.common.miscellaneous.check_dump_path")
    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_check_valid_acl_json_for_dump_fail4(
        self, mock_ms_open, mock_check_dump_path
    ):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = '{"dump": {"dump_list": [{"model_name": "model"}], "dump_path": "/path/to/dump", "dump_op_switch": "ccc}}'
        mock_check_dump_path.return_value = None
        with self.assertRaises(ValueError):
            check_valid_acl_json_for_dump("/path/to/acl.json", "/path/to/model.om")

    @patch("ais_bench.infer.common.miscellaneous.check_dump_path")
    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_check_valid_acl_json_for_dump_fail5(
        self, mock_ms_open, mock_check_dump_path
    ):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = '{"dump": {"dump_list": [{"model_name": "model"}], "dump_path": "/path/to/dump", "dump_mode": "ccc}}'
        mock_check_dump_path.return_value = None
        with self.assertRaises(ValueError):
            check_valid_acl_json_for_dump("/path/to/acl.json", "/path/to/model.om")

    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    @patch("ais_bench.infer.common.miscellaneous.makedirs_safe")
    def test_get_acl_json_path(self, mock_makedirs_safe, mock_ms_open):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = '{"profiler": {"switch": "on", "aicpu": "on", "output": "/path/to/profiler"}}'
        args = MagicMock()
        args.acl_json_path = None
        args.profiler = True
        args.output = "/path/to/output"
        args.model = "/path/to/model.om"
        acl_json_path = get_acl_json_path(args)
        self.assertEqual(acl_json_path, "/path/to/output/acl.json")

    @patch("ais_bench.infer.common.miscellaneous.check_valid_acl_json_for_dump")
    def test_get_acl_json_path1(self, mock_check_valid_acl_json_for_dump):
        mock_check_valid_acl_json_for_dump.return_value = None
        args = MagicMock()
        args.acl_json_path = "xxx"
        args.profiler = True
        args.output = "/path/to/output"
        args.model = "/path/to/model.om"
        acl_json_path = get_acl_json_path(args)
        self.assertEqual(acl_json_path, "xxx")

    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    @patch("ais_bench.infer.common.miscellaneous.makedirs_safe")
    @patch("ais_bench.infer.common.miscellaneous.check_valid_acl_json_for_dump")
    def test_get_acl_json_path2(
        self, mock_check_valid_acl_json_for_dump, mock_makedirs_safe, mock_ms_open
    ):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = '{"profiler": {"switch": "on", "aicpu": "on", "output": "/path/to/profiler"}}'
        mock_check_valid_acl_json_for_dump.return_value = None
        args = MagicMock()
        args.profiler = False
        args.dump = True
        args.output = "/path/to/output"
        args.model = "/path/to/model.om"
        args.acl_json_path = None
        acl_json_path = get_acl_json_path(args)
        self.assertEqual(acl_json_path, "/path/to/output/acl.json")

    def test_get_batchsize(self):
        session = MagicMock()
        session.get_inputs.return_value = [MagicMock(shape=[1, 2, 3])]
        args = MagicMock()
        args.dym_batch = 0
        args.dym_dims = None
        args.dym_shape = None
        batchsize = get_batchsize(session, args)
        self.assertEqual(batchsize, 1)

    def test_get_batchsize1(self):
        session = MagicMock()
        session.get_inputs.return_value = [MagicMock(shape=[1, 2, 3])]
        args = MagicMock()
        args.dym_batch = 2
        args.dym_dims = None
        args.dym_shape = None
        batchsize = get_batchsize(session, args)
        self.assertEqual(batchsize, 2)

    def test_get_batchsize2(self):
        session = MagicMock()
        session.get_inputs.return_value = [MagicMock(shape=[1, 2, 3])]
        args = MagicMock()
        args.dym_batch = 0
        args.dym_dims = "2;2:3"
        args.dym_shape = 3
        batchsize = get_batchsize(session, args)
        self.assertEqual(batchsize, 1)

    def test_get_range_list(self):
        ranges = "name1:1~10;name2:1,2,3"
        result = get_range_list(ranges)
        self.assertEqual(len(result), 10)

        ranges = "name1:1-10;name2:1,2,3"
        result = get_range_list(ranges)
        self.assertEqual(len(result), 2)

        ranges = "name1:1;name2:1,2,3"
        result = get_range_list(ranges)
        self.assertEqual(len(result), 1)

    @patch("os.path.isfile")
    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_get_dymshape_list(self, mock_ms_open, mock_isfile):
        mock_isfile.return_value = False
        mock_ms_open.return_value.__enter__.return_value.read.return_value = (
            "name1:1,2,3;name2:1~3"
        )
        dymshape_list = get_dymshape_list("input_ranges")
        self.assertEqual(len(dymshape_list), 1)

    @patch("os.path.isfile")
    @patch("ais_bench.infer.common.miscellaneous.ms_open")
    def test_get_dymshape_list1(self, mock_ms_open, mock_isfile):
        mock_isfile.return_value = True
        mock_ms_open.return_value.__enter__.return_value.read.return_value = (
            "name1:1,2,3;name2:1~3"
        )
        dymshape_list = get_dymshape_list("input_ranges")
        self.assertEqual(len(dymshape_list), 256)

    def test_get_throughtput_from_log(self):
        out_log = "throughput 100.0"
        result, throughput = get_throughtput_from_log(out_log)
        self.assertEqual(result, "OK")
        self.assertEqual(throughput, 100.0)

    # @patch('sys.executable')
    # @patch('ais_bench.infer.common.miscellaneous.AISBenchInferArgsAdapter.get_all_args_dict')
    def test_regenerate_dymshape_cmd(self):
        adapter = AISBenchInferArgsAdapter(
            model="model.onnx",
            input_path="input.bin",
            output="output.bin",
            output_dirname=None,
            outfmt="bin",
            loop="100",
            debug=True,
            device="npu",
            dym_batch="1",
            dym_hw="224,224",
            dym_dims="input:0",
            dym_shape="input:1,224,224,3",
            output_size="1000",
            auto_set_dymshape_mode="True",
            auto_set_dymdims_mode="False",
            batchsize="1",
            pure_data_type="fp32",
            profiler="",
            dump="True",
            acl_json_path="/path/to/acl.json",
            output_batchsize_axis="0",
            run_mode="infer",
            display_all_summary="True",
            warmup_count="5",
            dym_shape_range="input:1~8,224,224,3",
            aipp_config="config.cfg",
            energy_consumption="True",
            npu_id="0",
            backend="onnxruntime",
            perf="summary",
            pipeline="default",
            profiler_rename="rename",
            dump_npy="True",
            divide_input="False",
            threads="4",
        )
        new_dym_shape = "input:2,224,224,3"

        result = regenerate_dymshape_cmd(adapter, new_dym_shape)
        self.assertEqual(
            result[1:],
            [
                "-m",
                "ais_bench",
                "--model=model.onnx",
                "--input=input.bin",
                "--output=output.bin",
                "--outfmt=bin",
                "--loop=100",
                "--debug=True",
                "--device=npu",
                "--dymBatch=1",
                "--dymHW=224,224",
                "--dymDims=input:0",
                "--dymShape=input:2,224,224,3",
                "--outputSize=1000",
                "--auto_set_dymshape_mode=True",
                "--auto_set_dymdims_mode=False",
                "--batchsize=1",
                "--pure_data_type=fp32",
                "--dump=True",
                "--acl_json_path=/path/to/acl.json",
                "--output_batchsize_axis=0",
                "--run_mode=infer",
                "--display_all_summary=True",
                "--warmup_count=5",
                "--aipp_config=config.cfg",
                "--energy_consumption=True",
                "--npu_id=0",
                "--backend=onnxruntime",
                "--perf=summary",
                "--pipeline=default",
                "--profiler_rename=rename",
                "--dump_npy=True",
                "--divide_input=False",
                "--threads=4",
            ],
        )

    @patch("ais_bench.infer.common.miscellaneous.get_dymshape_list")
    @patch("ais_bench.infer.common.miscellaneous.regenerate_dymshape_cmd")
    @patch("ais_bench.infer.common.miscellaneous.subprocess.Popen")
    def test_dymshape_range_run(
        self, mock_popen, mock_regenerate_dymshape_cmd, mock_get_dymshape_list
    ):
        mock_get_dymshape_list.return_value = ["1,2,3"]
        mock_regenerate_dymshape_cmd.return_value = [
            "python",
            "-m",
            "ais_bench",
            "--dymShape=1,2,3",
        ]
        mock_popen.return_value.communicate.return_value = (b"throughput 100.0", b"")
        args = MagicMock()
        args.dym_shape_range = "1,2,3"
        dymshape_range_run(args)


if __name__ == "__main__":
    unittest.main()
