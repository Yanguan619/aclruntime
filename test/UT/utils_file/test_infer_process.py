import unittest
import queue
from unittest.mock import MagicMock, patch
from ais_bench.infer.infer_process import (
    set_session_options,
    init_inference_session,
    set_dymshape_shape,
    set_dymdims_shape,
    warmup,
    run_inference,
    run_pipeline_inference,
    infer_loop_tensor_run,
    infer_loop_files_run,
    infer_fulltensors_run,
    infer_loop_array_run,
    infer_pipeline_run,
    get_file_name,
    get_legal_json_content,
    json_to_msprof_cmd,
    regenerate_cmd,
    msprof_run_profiling,
    get_energy_consumption,
    convert,
    main,
    seg_input_data_for_multi_process,
    args_rules,
    acl_json_base_check,
    backend_run,
    infer_process,
    AISBenchInferArgsAdapter,
)


class TestInferenceFunctions(unittest.TestCase):
    def setUp(self):
        # Mock arguments
        self.args = MagicMock()
        self.args.dym_batch = 32
        self.args.dym_hw = None
        self.args.dym_dims = None
        self.args.dym_shape = None
        self.args.batchsize = 1
        self.args.auto_set_dymshape_mode = False
        self.args.auto_set_dymdims_mode = False
        self.args.aipp_config = None
        self.args.output_size = None
        self.args.device = 0
        self.args.model = "model.om"
        self.args.debug = False
        self.args.loop = 1

        self.real_args = AISBenchInferArgsAdapter(
            model=1,
            input_path=1,
            output=1,
            output_dirname=1,
            outfmt=1,
            loop=1,
            debug=1,
            device=1,
            dym_batch=1,
            dym_hw=1,
            dym_dims=1,
            dym_shape=1,
            output_size=1,
            auto_set_dymshape_mode=1,
            auto_set_dymdims_mode=1,
            batchsize=1,
            pure_data_type=1,
            profiler=1,
            dump=1,
            acl_json_path=1,
            output_batchsize_axis=1,
            run_mode=1,
            display_all_summary=1,
            warmup_count=1,
            dym_shape_range=1,
            aipp_config=1,
            energy_consumption=1,
            npu_id=1,
            backend=1,
            perf=1,
            pipeline=1,
            profiler_rename=1,
            dump_npy=1,
            divide_input=1,
            threads=1,
        )

        # Mock session
        self.session = MagicMock()
        self.session.set_dynamic_batchsize = MagicMock()
        self.session.set_dynamic_hw = MagicMock()
        self.session.set_dynamic_dims = MagicMock()
        self.session.set_dynamic_shape = MagicMock()
        self.session.set_staticbatch = MagicMock()
        self.session.get_max_dym_batchsize = MagicMock(return_value=32)
        self.session.get_dym_aipp_input_exist = MagicMock(return_value=0)
        self.session.load_aipp_config_file = MagicMock()
        self.session.check_dym_aipp_input_exist = MagicMock()
        self.session.set_custom_outsize = MagicMock()

    def test_set_session_options_dym_batch(self):
        set_session_options(self.session, self.args)
        self.session.set_dynamic_batchsize.assert_called_once_with(self.args.dym_batch)
        self.assertEqual(self.args.batchsize, 1)

    def test_set_session_options_dym_hw(self):
        self.args.dym_batch = 0
        self.args.dym_hw = "224,224"
        set_session_options(self.session, self.args)
        self.session.set_dynamic_hw.assert_called_once_with(224, 224)

    def test_set_session_options_dym_dims(self):
        self.args.dym_batch = 0
        self.args.dym_dims = "input0:1,224,224"
        set_session_options(self.session, self.args)
        self.session.set_dynamic_dims.assert_called_once_with(self.args.dym_dims)

    def test_set_session_options_dym_shape(self):
        self.args.dym_batch = 0
        self.args.dym_shape = "input0:1,224,224"
        set_session_options(self.session, self.args)
        self.session.set_dynamic_shape.assert_called_once_with(self.args.dym_shape)

    def test_set_session_options_static_batch(self):
        self.args.dym_batch = 0
        set_session_options(self.session, self.args)
        self.session.set_staticbatch.assert_called_once()

    def test_set_session_options_static_batch1(self):
        self.args.batchsize = None
        set_session_options(self.session, self.args)

    def test_set_session_options_static_batch2(self):
        self.args.aipp_config = 1
        self.session.get_dym_aipp_input_exist.return_value = 1
        set_session_options(self.session, self.args)
        self.session.check_dym_aipp_input_exist.assert_called_once()

    def test_set_session_options_static_batch3(self):
        self.args.aipp_config = None
        self.session.get_dym_aipp_input_exist.return_value = 1
        with self.assertRaises(RuntimeError):
            set_session_options(self.session, self.args)

    def test_set_session_options_static_batch4(self):
        self.session.get_dym_aipp_input_exist.return_value = 11
        with self.assertRaises(RuntimeError):
            set_session_options(self.session, self.args)

    def test_set_session_options_static_batch5(self):
        self.session.get_dym_aipp_input_exist.return_value = -1
        with self.assertRaises(RuntimeError):
            set_session_options(self.session, self.args)

    def test_set_session_options_static_batch6(self):
        self.args.aipp_config = 1
        self.session.get_dym_aipp_input_exist.return_value = 1
        self.args.output_size = 1

    @patch("ais_bench.infer.infer_process.InferSession")
    @patch("ais_bench.infer.infer_process.set_session_options")
    def test_init_inference_session(self, mock_set_session_options, mock_InferSession):
        acl_json_path = "acl.json"
        # Mock InferSession
        mock_session = MagicMock()
        mock_InferSession.return_value = mock_session

        # Call function
        session = init_inference_session(self.args, acl_json_path)

        # Assert
        self.assertEqual(session, mock_session)
        mock_InferSession.assert_called_once_with(
            self.args.device,
            self.args.model,
            acl_json_path,
            self.args.debug,
            self.args.loop,
        )
        mock_set_session_options.assert_called_once_with(session, self.args)

    @patch("ais_bench.infer.infer_process.logger")
    def test_set_dymshape_shape(self, mock_logger):
        # Mock session and inputs
        session = MagicMock()
        inputs = [MagicMock(shape=[1, 224, 224, 3])]
        intensors_desc = [MagicMock(name="input0", realsize=1 * 224 * 224 * 3)]

        # Mock session methods
        session.get_inputs.return_value = intensors_desc

        # Call function
        with self.assertRaises(ValueError):
            set_dymshape_shape(self.session, inputs)

    @patch("ais_bench.infer.infer_process.logger")
    def test_set_dymshape_shape1(self, mock_logger):
        # Mock session and inputs
        session = MagicMock()
        inputs = [MagicMock(shape=[1, 224, 224, 3])]
        intensors_desc = [MagicMock(name="input0", realsize=1 * 224 * 224 * 3)]

        # Mock session methods
        session.get_inputs.return_value = intensors_desc

        # Call function
        set_dymshape_shape(session, inputs)
        session.set_dynamic_shape.assert_called_once()

    @patch("ais_bench.infer.infer_process.logger")
    def test_set_dymdims_shape(self, mock_logger):
        # Mock session and inputs
        session = MagicMock()
        inputs = [MagicMock(shape=[1, 224, 224, 3])]
        intensors_desc = [MagicMock(name="input0", realsize=1 * 224 * 224 * 3)]

        # Mock session methods
        session.get_inputs.return_value = intensors_desc

        # Call function
        set_dymdims_shape(session, inputs)

        # Assert
        session.set_dynamic_dims.assert_called_once()

    @patch("ais_bench.infer.infer_process.get_tensor_from_files_list")
    @patch("ais_bench.infer.infer_process.get_narray_from_files_list")
    @patch("ais_bench.infer.infer_process.run_inference")
    def test_warmup(
        self,
        mock_run_inference,
        mock_get_narray_from_files_list,
        mock_get_tensor_from_files_list,
    ):
        # Mock session and arguments
        MagicMock()
        intensors_desc = [MagicMock(realsize=1 * 224 * 224 * 3)]
        infiles = [["file1"], ["file2"]]

        # Mock methods
        mock_get_tensor_from_files_list.return_value = MagicMock()
        mock_get_narray_from_files_list.return_value = MagicMock()
        mock_run_inference.return_value = [MagicMock()]

        # Call function
        with self.assertRaises(ValueError):
            warmup(self.session, self.args, intensors_desc, infiles)

    @patch("ais_bench.infer.infer_process.get_tensor_from_files_list")
    @patch("ais_bench.infer.infer_process.get_narray_from_files_list")
    @patch("ais_bench.infer.infer_process.run_inference")
    def test_warmup1(
        self,
        mock_run_inference,
        mock_get_narray_from_files_list,
        mock_get_tensor_from_files_list,
    ):
        # Mock session and arguments
        session = MagicMock()
        intensors_desc = [MagicMock(realsize=1 * 224 * 224 * 3)]
        infiles = [["file1"]]

        # Mock methods
        mock_get_tensor_from_files_list.return_value = MagicMock()
        mock_get_narray_from_files_list.return_value = MagicMock()
        mock_run_inference.return_value = [MagicMock()]

        # Call function
        warmup(session, self.args, intensors_desc, infiles)
        session.set_loop_count.assert_called_with(1)

    @patch("ais_bench.infer.infer_process.set_dymshape_shape")
    @patch("ais_bench.infer.infer_process.set_dymdims_shape")
    def test_run_inference(self, mock_set_dymdims_shape, mock_set_dymshape_shape):
        # Mock session and arguments
        inputs = [MagicMock()]
        run_inference(self.session, self.args, inputs)
        self.session.run.assert_called_once()

    @patch("ais_bench.infer.infer_process.set_dymshape_shape")
    @patch("ais_bench.infer.infer_process.set_dymdims_shape")
    def test_run_inference1(self, mock_set_dymdims_shape, mock_set_dymshape_shape):
        # Mock session and arguments
        inputs = [MagicMock()]
        mock_set_dymshape_shape.return_value = None
        self.args.auto_set_dymshape_mode = True
        run_inference(self.session, self.args, inputs)
        self.session.run.assert_called_once()

    @patch("ais_bench.infer.infer_process.set_dymshape_shape")
    @patch("ais_bench.infer.infer_process.set_dymdims_shape")
    def test_run_inference2(self, mock_set_dymdims_shape, mock_set_dymshape_shape):
        # Mock session and arguments
        inputs = [MagicMock()]
        mock_set_dymdims_shape.return_value = None
        self.args.auto_set_dymdims_mode = True
        run_inference(self.session, self.args, inputs)
        self.session.run.assert_called_once()

    @patch("ais_bench.infer.infer_process.logger")
    def test_run_pipeline_inference(self, mock_logger):
        infileslist = [["file1", "file2"], ["file3", "file4"]]
        output_prefix = "output"
        extra_session = [MagicMock(session=MagicMock())]

        # Call function
        run_pipeline_inference(
            self.session, self.args, infileslist, output_prefix, extra_session
        )

        # Assert
        self.session.run_pipeline.assert_called()

    @patch("ais_bench.infer.infer_process.tqdm")
    @patch("ais_bench.infer.infer_process.run_inference")
    @patch("ais_bench.infer.infer_process.save_tensors_to_file")
    @patch("ais_bench.infer.infer_process.get_tensor_from_files_list")
    def test_infer_loop_tensor_run(
        self,
        mock_get_tensor_from_files_list,
        mock_save_tensors_to_file,
        mock_run_inference,
        mock_tqdm,
    ):
        # Mock session and args
        mock_get_tensor_from_files_list.return_value = 1
        session = MagicMock()
        args = MagicMock()
        args.pure_data_type = "zero"
        args.no_combine_tensor_mode = False
        args.outfmt = "npy"
        args.output_batchsize_axis = 0

        # Mock tqdm
        mock_tqdm.return_value = [(0, ["file1", "file2"])]

        # Mock run_inference
        mock_run_inference.return_value = [MagicMock()]

        # Mock save_tensors_to_file
        mock_save_tensors_to_file.return_value = None

        # Mock intensors_desc
        intensors_desc = [MagicMock(realsize=10), MagicMock(realsize=10)]

        # Mock infileslist
        infileslist = [["file1.txt", "file1.txt"]]

        # Call function
        infer_loop_tensor_run(
            session, args, intensors_desc, infileslist, output_prefix="output"
        )

        # Assert
        mock_run_inference.assert_called_once()
        mock_save_tensors_to_file.assert_called_once()

    @patch("ais_bench.infer.infer_process.tqdm")
    @patch("ais_bench.infer.infer_process.run_inference")
    @patch("ais_bench.infer.infer_process.save_tensors_to_file")
    def test_infer_loop_files_run(
        self, mock_save_tensors_to_file, mock_run_inference, mock_tqdm
    ):
        # Mock session and args
        session = MagicMock()
        args = MagicMock()
        args.outfmt = "npy"
        args.output_batchsize_axis = 0

        # Mock tqdm
        mock_tqdm.return_value = [(0, ["file1", "file2"])]

        # Mock run_inference
        mock_run_inference.return_value = [MagicMock()]

        # Mock save_tensors_to_file
        mock_save_tensors_to_file.return_value = None

        # Mock intensors_desc
        intensors_desc = [MagicMock()]

        # Mock infileslist
        infileslist = [["file1.txt", "file1.txt"]]

        # Call function
        with self.assertRaises(ValueError):
            infer_loop_files_run(
                session, args, intensors_desc, infileslist, output_prefix="output"
            )

    @patch("ais_bench.infer.infer_process.tqdm")
    @patch("ais_bench.infer.infer_process.run_inference")
    @patch("ais_bench.infer.infer_process.save_tensors_to_file")
    def test_infer_loop_files_run1(
        self, mock_save_tensors_to_file, mock_run_inference, mock_tqdm
    ):
        # Mock session and args
        session = MagicMock()
        args = MagicMock()
        args.outfmt = "npy"
        args.output_batchsize_axis = 0

        # Mock tqdm
        mock_tqdm.return_value = [["file1", "file2"]]

        # Mock run_inference
        mock_run_inference.return_value = [MagicMock()]

        # Mock save_tensors_to_file
        mock_save_tensors_to_file.return_value = None

        # Mock intensors_desc
        intensors_desc = ["file1", "file2"]

        # Mock infileslist
        infileslist = [["file1.txt", "file1.txt"]]

        # Call function
        infer_loop_files_run(
            session, args, intensors_desc, infileslist, output_prefix="output"
        )

        session.convert_tensors_to_host.assert_called_once()

    @patch("ais_bench.infer.infer_process.tqdm")
    @patch("ais_bench.infer.infer_process.run_inference")
    @patch("ais_bench.infer.infer_process.save_tensors_to_file")
    @patch("ais_bench.infer.infer_process.create_intensors_from_infileslist")
    def test_infer_fulltensors_run(
        self,
        mock_create_intensors_from_infileslist,
        mock_save_tensors_to_file,
        mock_run_inference,
        mock_tqdm,
    ):
        # Mock session and args
        session = MagicMock()
        args = MagicMock()
        args.pure_data_type = "zero"
        args.no_combine_tensor_mode = False
        args.outfmt = "npy"
        args.output_batchsize_axis = 0
        mock_create_intensors_from_infileslist.return_value = ["file1", "file2"]

        # Mock tqdm
        mock_tqdm.return_value = [(0, ["file1", "file2"])]

        # Mock run_inference
        mock_run_inference.return_value = [MagicMock()]

        # Mock save_tensors_to_file
        mock_save_tensors_to_file.return_value = None

        # Mock intensors_desc
        intensors_desc = [MagicMock(realsize=10), MagicMock(realsize=10)]

        # Mock infileslist
        infileslist = [["file1.txt", "file1.txt"]]
        infer_fulltensors_run(
            session, args, intensors_desc, infileslist, output_prefix="output"
        )
        mock_run_inference.assert_called_once()
        mock_save_tensors_to_file.assert_called_once()

    @patch("ais_bench.infer.infer_process.tqdm")
    @patch("ais_bench.infer.infer_process.run_inference")
    @patch("ais_bench.infer.infer_process.save_tensors_to_file")
    @patch("ais_bench.infer.infer_process.get_narray_from_files_list")
    def test_infer_loop_array_run(
        self,
        mock_get_narray_from_files_list,
        mock_save_tensors_to_file,
        mock_run_inference,
        mock_tqdm,
    ):
        session = MagicMock()
        args = MagicMock()
        args.pure_data_type = "zero"
        args.outfmt = "npy"
        args.output_batchsize_axis = 0
        mock_get_narray_from_files_list.return_value = [1, 2]
        mock_tqdm.return_value = [(0, ["file1", "file2"])]
        mock_run_inference.return_value = [MagicMock()]
        mock_save_tensors_to_file.return_value = None
        intensors_desc = [MagicMock(realsize=10), MagicMock(realsize=10)]
        infileslist = [["file1.txt", "file1.txt"]]

        infer_loop_array_run(
            session, args, intensors_desc, infileslist, output_prefix="output"
        )

        mock_run_inference.assert_called_once()
        mock_save_tensors_to_file.assert_called_once()

    @patch("ais_bench.infer.infer_process.run_pipeline_inference")
    def test_infer_pipeline_run(self, mock_run_pipeline_inference):
        session = MagicMock()
        args = MagicMock()
        args.threads = 2
        mock_run_pipeline_inference.return_value = None
        infileslist = [["file1.txt", "file1.txt"]]
        extra_session = [MagicMock(), MagicMock()]

        infer_pipeline_run(
            session,
            args,
            infileslist,
            output_prefix="output",
            extra_session=extra_session,
        )

        mock_run_pipeline_inference.assert_called_once()

    @patch("ais_bench.infer.infer_process.os.walk")
    def test_get_file_name(self, mock_os_walk):
        mock_os_walk.return_value = [
            ("/path/to/files", [], ["file1.txt", "file2.txt"]),
        ]
        res_file_path = []

        get_file_name("/path/to/files", ".txt", res_file_path)

        self.assertEqual(
            res_file_path, ["/path/to/files/file1.txt", "/path/to/files/file2.txt"]
        )

    @patch("ais_bench.infer.infer_process.ms_open")
    def test_get_legal_json_content(self, mock_ms_open):
        mock_ms_open.return_value.__enter__.return_value.read.return_value = (
            '{"profiler": {"output": "/path/to/output"}}'
        )
        acl_json_path = "acl.json"

        cmd_dict = get_legal_json_content(acl_json_path)

        self.assertEqual(cmd_dict, {"--output": "/path/to/output"})

    @patch("ais_bench.infer.infer_process.get_legal_json_content")
    def test_json_to_msprof_cmd(self, mock_get_legal_json_content):
        acl_json_path = "acl.json"
        mock_get_legal_json_content.return_value = {
            "--acl_json_path": "xxxx",
            "--warmup_count": 1,
            "--profiler": 1,
        }

        msprof_cmd = json_to_msprof_cmd(acl_json_path)

        self.assertIn("--acl_json_path=xxxx", msprof_cmd)

    def test_regenerate_cmd(self):
        cmd = regenerate_cmd(self.real_args)
        self.assertIn("--profiler=0", cmd)

    @patch("os.rename")
    @patch("subprocess.Popen")
    @patch("subprocess.call")
    @patch("fcntl.fcntl")
    @patch("ais_bench.infer.infer_process.regenerate_cmd")
    @patch("ais_bench.infer.infer_process.json_to_msprof_cmd")
    @patch("ais_bench.infer.infer_process.get_file_name")
    def test_msprof_run_profiling(
        self,
        mock_get_file_name,
        mock_json_to_msprof_cmd,
        mock_regenerate_cmd,
        mock_fcntl,
        mock_call,
        mock_Popen,
        mock_rename,
    ):
        args = MagicMock()
        args.acl_json_path = None
        args.profiler_rename = True
        args.output = "path/to/output"
        args.model = "model.om"
        msprof_bin = "msprof"
        mock_regenerate_cmd.return_value = "python -m ais_bench"
        mock_json_to_msprof_cmd.return_value = "--output=path/to/output/profiler"
        mock_fcntl.return_value = 0
        mock_get_file_name.return_value = ["file1.txt", "file2.txt"]
        mock_rename.return_value = None
        mock_process = MagicMock()
        mock_process.stdout.read.side_effect = [b"PROF_XXXX", b""]
        mock_Popen.return_value = mock_process

        ret = msprof_run_profiling(args, msprof_bin)

        self.assertEqual(ret, 0)

    @patch("subprocess.Popen")
    @patch("subprocess.call")
    @patch("fcntl.fcntl")
    @patch("ais_bench.infer.infer_process.regenerate_cmd")
    @patch("ais_bench.infer.infer_process.json_to_msprof_cmd")
    def test_msprof_run_profiling_no_rename(
        self,
        mock_json_to_msprof_cmd,
        mock_regenerate_cmd,
        mock_fcntl,
        mock_call,
        mock_Popen,
    ):
        args = MagicMock()
        args.acl_json_path = None
        args.profiler_rename = False
        args.output = "path/to/output"
        args.model = "model.om"
        msprof_bin = "msprof"
        mock_regenerate_cmd.return_value = "python -m ais_bench"
        mock_json_to_msprof_cmd.return_value = ""
        mock_fcntl.return_value = 0

        msprof_run_profiling(args, msprof_bin)

        mock_regenerate_cmd.assert_called_once_with(args)
        mock_json_to_msprof_cmd.assert_not_called()

    @patch("subprocess.run")
    def test_get_energy_consumption(self, mock_run):
        npu_id = 0
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = b"Power Dissipation(W): 100\n"
        mock_run.return_value = mock_result

        power = get_energy_consumption(npu_id)

        self.assertEqual(power, "")

    @patch("subprocess.run")
    def test_get_energy_consumption_invalid_npu_id(self, mock_run):
        npu_id = 0
        mock_result = MagicMock()
        mock_result.returncode = 1
        mock_result.stdout = b""
        mock_result.stderr = b"Invalid npu id"
        mock_run.return_value = mock_result

        with self.assertRaises(RuntimeError):
            get_energy_consumption(npu_id)

    @patch("shutil.rmtree")
    @patch("os.remove")
    @patch("ais_bench.infer.infer_process.move_subdir")
    @patch("ais_bench.infer.infer_process.convert_helper")
    @patch("ais_bench.infer.infer_process.check_path_legality")
    def test_convert(
        self,
        mock_check_path_legality,
        mock_convert_helper,
        mock_move_subdir,
        mock_remove,
        mock_rmtree,
    ):
        tmp_acl_json_path = "path/to/tmp_acl.json"
        real_dump_path = "path/to/real_dump"
        tmp_dump_path = "path/to/tmp_dump"
        mock_move_subdir.return_value = ("output_dir", "timestamp")
        mock_check_path_legality.return_value = None

        convert(tmp_acl_json_path, real_dump_path, tmp_dump_path)

        mock_move_subdir.assert_called_once_with(tmp_dump_path, real_dump_path)
        mock_convert_helper.assert_called_once_with("output_dir", "timestamp")
        mock_rmtree.assert_called_once_with(tmp_dump_path)
        mock_remove.assert_called_once_with(tmp_acl_json_path)
        mock_check_path_legality.assert_called()

    @patch("ais_bench.infer.infer_process.logger")
    @patch("ais_bench.infer.infer_process.get_acl_json_path")
    @patch("ais_bench.infer.infer_process.create_tmp_acl_json")
    @patch("ais_bench.infer.infer_process.init_inference_session")
    @patch("ais_bench.infer.infer_process.warmup")
    @patch("ais_bench.infer.infer_process.infer_pipeline_run")
    @patch("ais_bench.infer.infer_process.infer_loop_array_run")
    @patch("ais_bench.infer.infer_process.infer_loop_files_run")
    @patch("ais_bench.infer.infer_process.infer_fulltensors_run")
    @patch("ais_bench.infer.infer_process.infer_loop_tensor_run")
    @patch("ais_bench.infer.infer_process.summary")
    @patch("ais_bench.infer.infer_process.MemorySummary")
    @patch("ais_bench.infer.infer_process.convert")
    @patch("ais_bench.infer.infer_process.get_energy_consumption")
    def test_main_normal(
        self,
        mock_get_energy_consumption,
        mock_convert,
        mock_MemorySummary,
        mock_Summary,
        mock_infer_loop_tensor_run,
        mock_infer_fulltensors_run,
        mock_infer_loop_files_run,
        mock_infer_loop_array_run,
        mock_infer_pipeline_run,
        mock_warmup,
        mock_init_inference_session,
        mock_create_tmp_acl_json,
        mock_get_acl_json_path,
        mock_logger,
    ):
        self.args.threads = 1
        session = MagicMock()
        mock_init_inference_session.return_value = session
        intensors_desc = [MagicMock()]
        session.get_inputs.return_value = intensors_desc
        [[MagicMock()]]
        summary = MagicMock()
        mock_Summary.return_value = summary
        memory_summary = MagicMock()
        mock_MemorySummary.return_value = memory_summary
        mock_get_energy_consumption.return_value = 1
        mock_get_acl_json_path.return_value = "acl.json"
        mock_create_tmp_acl_json.return_value = (
            "tmp_acl.json",
            "real_dump_path",
            "tmp_dump_path",
        )
        mock_warmup.return_value = None
        mock_infer_pipeline_run.return_value = None
        mock_infer_loop_array_run.return_value = None
        mock_infer_loop_files_run.return_value = None
        mock_infer_fulltensors_run.return_value = None
        mock_infer_loop_tensor_run.return_value = None
        mock_convert.return_value = None

        main(self.args)

        mock_init_inference_session.assert_called()

    @patch("ais_bench.infer.infer_process.logger")
    @patch("ais_bench.infer.infer_process.get_acl_json_path")
    @patch("ais_bench.infer.infer_process.create_tmp_acl_json")
    @patch("ais_bench.infer.infer_process.init_inference_session")
    @patch("ais_bench.infer.infer_process.warmup")
    @patch("ais_bench.infer.infer_process.infer_pipeline_run")
    @patch("ais_bench.infer.infer_process.infer_loop_array_run")
    @patch("ais_bench.infer.infer_process.infer_loop_files_run")
    @patch("ais_bench.infer.infer_process.infer_fulltensors_run")
    @patch("ais_bench.infer.infer_process.infer_loop_tensor_run")
    @patch("ais_bench.infer.infer_process.summary")
    @patch("ais_bench.infer.infer_process.MemorySummary")
    @patch("ais_bench.infer.infer_process.convert")
    @patch("ais_bench.infer.infer_process.get_energy_consumption")
    @patch("ais_bench.infer.infer_process.create_pipeline_fileslist_from_inputs_list")
    @patch("ais_bench.infer.infer_process.check_path_legality")
    def test_main_normal1(
        self,
        mock_check_path_legality,
        mock_create_pipeline_fileslist_from_inputs_list,
        mock_get_energy_consumption,
        mock_convert,
        mock_MemorySummary,
        mock_Summary,
        mock_infer_loop_tensor_run,
        mock_infer_fulltensors_run,
        mock_infer_loop_files_run,
        mock_infer_loop_array_run,
        mock_infer_pipeline_run,
        mock_warmup,
        mock_init_inference_session,
        mock_create_tmp_acl_json,
        mock_get_acl_json_path,
        mock_logger,
    ):
        self.args.threads = 1
        self.args.output = "xx"
        self.args.output_dirname = "yy"
        self.args.input = "xx"
        self.args.subprocess_count = 0
        session = MagicMock()
        mock_init_inference_session.return_value = session
        intensors_desc = [MagicMock()]
        session.get_inputs.return_value = intensors_desc
        [[MagicMock()]]
        summary = MagicMock()
        mock_create_pipeline_fileslist_from_inputs_list.return_value = ["file1"]
        mock_Summary.return_value = summary
        memory_summary = MagicMock()
        mock_MemorySummary.return_value = memory_summary
        mock_get_energy_consumption.return_value = 1
        mock_get_acl_json_path.return_value = "acl.json"
        mock_create_tmp_acl_json.return_value = (
            "tmp_acl.json",
            "real_dump_path",
            "tmp_dump_path",
        )
        mock_warmup.return_value = None
        mock_check_path_legality.return_value = None
        mock_infer_pipeline_run.return_value = None
        mock_infer_loop_array_run.return_value = None
        mock_infer_loop_files_run.return_value = None
        mock_infer_fulltensors_run.return_value = None
        mock_infer_loop_tensor_run.return_value = None
        mock_convert.return_value = None
        qq = queue.Queue()
        qq.put(1)

        main(self.args, msgq=qq, device_list=[1, 2])

        mock_init_inference_session.assert_called()

    @patch("ais_bench.infer.infer_process.get_fileslist_from_dir")
    @patch("ais_bench.infer.infer_process.init_inference_session")
    @patch("ais_bench.infer.infer_process.get_acl_json_path")
    @patch("ais_bench.infer.infer_process.list_share")
    @patch("os.path.isfile")
    def test_seg_input_data_for_multi_process(
        self,
        mock_isfile,
        mock_list_share,
        mock_get_acl_json_path,
        mock_init_inference_session,
        mock_get_fileslist_from_dir,
    ):
        # Mock inputs
        inputs = "path/to/input1,path/to/input2"
        jobs = 2

        # Mock get_fileslist_from_dir
        mock_get_fileslist_from_dir.return_value = ["file1", "file2", "file3", "file4"]

        # Mock init_inference_session
        session = MagicMock()
        session.get_inputs.return_value = [MagicMock(), MagicMock()]
        mock_init_inference_session.return_value = session
        mock_isfile.return_value = True
        mock_get_acl_json_path.return_value = "xcx"
        mock_list_share.return_value = []

        # Call function
        result = seg_input_data_for_multi_process(self.args, inputs, jobs)

        # Assert
        self.assertEqual(len(result), jobs)

    def test_args_rules(self):
        # Test profiler and dump conflict
        self.args.profiler = True
        self.args.dump = True
        with self.assertRaises(RuntimeError):
            args_rules(self.args)

        self.args.output_dirname = "/absolute/path"
        self.args.dump = False
        with self.assertRaises(ValueError):
            args_rules(self.args)

        self.args.output_dirname = "xx"
        self.args.output = None
        with self.assertRaises(RuntimeError):
            args_rules(self.args)

        self.args.warmup_count = 1
        self.args.input = 2
        self.args.profiler = False

        self.args.output_dirname = "relative/path"
        with self.assertRaises(RuntimeError):
            args_rules(self.args)

        self.args.output_dirname = None
        self.args.threads = 2
        self.args.pipeline = False

        self.assertEqual(args_rules(self.args), self.args)

    @patch("ais_bench.infer.infer_process.ms_open")
    @patch("json.load")
    def test_acl_json_base_check_normal(self, mock_json, mock_ms_open):
        # Mock args
        self.args.acl_json_path = "path/to/acl.json"
        self.args.profiler = False
        self.args.dump = False

        # Mock ms_open and json.load
        mock_ms_open.return_value.__enter__.return_value.read.return_value = (
            '{"profiler": {"switch": "on"}}'
        )
        mock_json.return_value = {"profiler": {"switch": "on"}}

        # Call function
        updated_args = acl_json_base_check(self.args)

        # Assert
        self.assertTrue(updated_args.profiler)
        self.assertFalse(updated_args.dump)

    @patch(
        "ais_bench.infer.infer_process.ms_open",
        side_effect=Exception("Mocked exception"),
    )
    def test_acl_json_base_check_exception(self, mock_ms_open):
        # Mock args
        self.args.acl_json_path = "path/to/acl.json"

        # Call function and assert exception
        with self.assertRaises(Exception):
            acl_json_base_check(self.args)

    @patch("ais_bench.infer.infer_process.BackendFactory")
    def test_backend_run_normal(self, mock_BackendFactory):
        self.args.backend = "mock_backend"
        self.args.model = "model.om"

        mock_backend_class = MagicMock()
        mock_BackendFactory.create_backend.return_value = mock_backend_class
        mock_backend = mock_backend_class.return_value
        mock_backend.load.return_value = None
        mock_backend.run.return_value = None
        mock_backend.get_perf.return_value = "perf_info"

        backend_run(self.args)

        mock_BackendFactory.create_backend.assert_called_once_with(self.args.backend)
        mock_backend.load.assert_called_once_with(self.args.model)
        mock_backend.run.assert_called_once()
        mock_backend.get_perf.assert_called_once()

    @patch("ais_bench.infer.infer_process.args_rules")
    @patch("ais_bench.infer.infer_process.version_check")
    @patch("ais_bench.infer.infer_process.acl_json_base_check")
    @patch("ais_bench.infer.infer_process.backend_run")
    @patch("ais_bench.infer.infer_process.get_msprof_bin_path")
    @patch("ais_bench.infer.infer_process.msprof_run_profiling")
    @patch("ais_bench.infer.infer_process.dymshape_range_run")
    @patch("ais_bench.infer.infer_process.multidevice_run")
    @patch("ais_bench.infer.infer_process.main")
    def test_infer_process_normal(
        self,
        mock_main,
        mock_multidevice_run,
        mock_dymshape_range_run,
        mock_msprof_run_profiling,
        mock_get_msprof_bin_path,
        mock_backend_run,
        mock_acl_json_base_check,
        mock_version_check,
        mock_args_rules,
    ):
        self.args.perf = False
        self.args.profiler = False
        self.args.dym_shape_range = None
        self.args.dym_shape = None
        self.args.device = 0

        mock_args_rules.return_value = self.args
        mock_version_check.return_value = None
        mock_acl_json_base_check.return_value = self.args
        mock_get_msprof_bin_path.return_value = "msprof"
        mock_msprof_run_profiling.return_value = 0
        mock_dymshape_range_run.return_value = 0
        mock_multidevice_run.return_value = 0
        mock_main.return_value = 0

        ret = infer_process(self.args)
        self.assertEqual(ret, 0)

        self.args.perf = True
        mock_backend_run.return_value = None
        self.assertEqual(ret, 0)

        self.args.perf = False
        self.args.profiler = True
        self.assertEqual(ret, 0)

        self.args.profiler = False
        self.args.dym_shape_range = 1
        self.assertEqual(ret, 0)

        self.args.dym_shape_range = None
        self.args.device = [1]
        self.assertEqual(ret, 0)


if __name__ == "__main__":
    unittest.main()
