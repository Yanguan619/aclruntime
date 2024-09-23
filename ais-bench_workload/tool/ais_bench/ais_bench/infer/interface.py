# Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import time
from multiprocessing import Pool
from multiprocessing import Manager
import numpy as np
import aclruntime
from ais_bench.infer.common.logger import logger
from ais_bench.infer.dym_aipp_manager import DymAippManager
from ais_bench.infer.interface_check import (check_model_path_legality, check_acl_json_path_legality,
    check_device_range_valid, check_positive_integer, check_custom_size, check_bool_value, 
    check_in_out_list, check_loop_size)

TORCH_TENSOR_LIST = [
    'torch.FloatTensor', 'torch.DoubleTensor', 'torch.HalfTensor', 'torch.BFloat16Tensor',
    'torch.ByteTensor', 'torch.CharTensor', 'torch.ShortTensor', 'torch.LongTensor',
    'torch.BoolTensor', 'torch.IntTensor'
]
NP_TYPE_LIST = [
    np.int8, np.int16, np.int32, np.int64, np.uint8, np.uint16,
    np.uint32, np.float16, np.float32, np.float64
]


class InferSession:
    def __init__(self, device_id: int, model_path: str, acl_json_path: str = None,
                 debug: bool = False, loop: int = 1):
        """
        init InferSession

        Args:
            device_id: device id for npu device
            model_path: om model path to load
            acl_json_path: set acl_json_path to enable profiling or dump function
            debug: enable debug log.  Default: False
            loop: loop count for one inference. Default: 1
        """
        check_model_path_legality(model_path)
        check_acl_json_path_legality(acl_json_path)
        check_device_range_valid(device_id)
        check_loop_size(loop)
        self.device_id = device_id
        self.model_path = model_path
        self.loop = loop
        self.options = aclruntime.session_options()
        self.acl_json_path = acl_json_path
        self.debug = debug
        if acl_json_path is not None:
            self.options.acl_json_path = self.acl_json_path
        self.options.log_level = 1 if self.debug else 2
        self.options.loop = self.loop
        self.session = aclruntime.InferenceSession(self.model_path, self.device_id, self.options)
        self.outputs_names = [meta.name for meta in self.session.get_outputs()]
        self.intensors_desc = self.session.get_inputs()
        self.outtensors_desc = self.session.get_outputs()
        self.infer_mode_switch = {
            "static": self._static_prepare,
            "dymbatch": self._dymbatch_prepare,
            "dymhw": self._dymhw_prepare,
            "dymdims": self._dymdims_prepare,
            "dymshape": self._dymshape_prepare
        }

    @staticmethod
    def convert_tensors_to_host(tensors):
        for tensor in tensors:
            tensor.to_host()

    @staticmethod
    def convert_tensors_to_arrays(tensors):
        arrays = []
        for tensor in tensors:
            # convert acltensor to numpy array
            arrays.append(np.array(tensor))
        return arrays

    @staticmethod
    def finalize():
        if hasattr(aclruntime.InferenceSession, 'finalize'):
            aclruntime.InferenceSession.finalize()

    def get_inputs(self):
        """
        get inputs info of model
        """
        self.intensors_desc = self.session.get_inputs()
        return self.intensors_desc

    def get_outputs(self):
        """
        get outputs info of model
        """
        self.outtensors_desc = self.session.get_outputs()
        return self.outtensors_desc

    def set_loop_count(self, loop):
        options = self.session.options()
        options.loop = loop

    def set_context(self):
        self.session.set_context()

    # 默认设置为静态batch
    def set_staticbatch(self):
        self.session.set_staticbatch()

    def set_dynamic_batchsize(self, dym_batch: str):
        self.session.set_dynamic_batchsize(dym_batch)

    def set_dynamic_hw(self, w: int, h: int):
        self.session.set_dynamic_hw(w, h)

    def get_max_dym_batchsize(self):
        return self.session.get_max_dym_batchsize()

    def set_dynamic_dims(self, dym_dims: str):
        self.session.set_dynamic_dims(dym_dims)

    def set_dynamic_shape(self, dym_shape: str):
        self.session.set_dynamic_shape(dym_shape)

    def set_custom_outsize(self, custom_sizes):
        self.session.set_custom_outsize(custom_sizes)

    def create_tensor_from_fileslist(self, desc, files):
        return self.session.create_tensor_from_fileslist(desc, files)

    def create_tensor_from_arrays_to_device(self, arrays):
        tensor = aclruntime.Tensor(arrays)
        tensor.to_device(self.device_id)
        return tensor

    def get_dym_aipp_input_exist(self):
        return self.session.get_dym_aipp_input_exist()

    def check_dym_aipp_input_exist(self):
        self.session.check_dym_aipp_input_exist()

    def load_aipp_config_file(self, config_file, batchsize):
        aipp_manager = DymAippManager(self.session, config_file, batchsize)
        aipp_manager.load_aipp_config_content()
        ret = self.session.set_dym_aipp_info_set()
        return ret

    def run(self, feeds, out_array=False):
        if len(feeds) > 0 and isinstance(feeds[0], np.ndarray):
            # if feeds is ndarray list, convert to baseTensor
            inputs = []
            for array in feeds:
                basetensor = aclruntime.BaseTensor(array.__array_interface__['data'][0], array.nbytes)
                inputs.append(basetensor)
        else:
            inputs = feeds
        outputs = self.session.run(self.outputs_names, inputs)
        if out_array:
            # convert to host tensor
            self.convert_tensors_to_host(outputs)
            # convert tensor to narray
            return self.convert_tensors_to_arrays(outputs)
        else:
            return outputs

    def run_pipeline(self, infilelist, output, auto_shape=False,
                     auto_dims=False, outfmt="BIN", pure_infer_mode=False, extra_session=None):
        infer_options = aclruntime.infer_options()
        infer_options.output_dir = output
        infer_options.auto_dym_shape = auto_shape
        infer_options.auto_dym_dims = auto_dims
        infer_options.out_format = outfmt
        infer_options.pure_infer_mode = pure_infer_mode
        extra_session = [] if extra_session is None else extra_session
        self.session.run_pipeline(infilelist, infer_options, extra_session)

    def reset_summaryinfo(self):
        self.session.reset_sumaryinfo()

    def infer(self, feeds, mode='static', custom_sizes=100000, out_array=True):
        '''
        Parameters:
            feeds: input data
            mode: static dymdims dymshape...
        '''
        inputs = []
        shapes = []
        check_bool_value(out_array)
        check_custom_size(custom_sizes, mode)
        for feed in feeds:
            if type(feed) is np.ndarray:
                infer_input = feed
                if not infer_input.flags.c_contiguous:
                    infer_input = np.ascontiguousarray(infer_input)
                shapes.append(infer_input.shape)
            elif type(feed) in NP_TYPE_LIST:
                infer_input = np.array(feed)
                if not infer_input.flags.c_contiguous:
                    infer_input = np.ascontiguousarray(infer_input)
                shapes.append([feed.size])
            elif type(feed) is aclruntime.Tensor:
                infer_input = feed
                shapes.append(infer_input.shape)
            elif hasattr(feed, 'type') and feed.type() in TORCH_TENSOR_LIST:
                infer_input = feed.numpy()
                if not feed.is_contiguous():
                    infer_input = np.ascontiguousarray(infer_input)
                shapes.append(infer_input.shape)
            else:
                raise RuntimeError('type:{} invalid'.format(type(feed)))
            inputs.append(infer_input)

        if self.infer_mode_switch.get(mode) is not None:
            self.infer_mode_switch.get(mode)(shapes, custom_sizes)
        else:
            raise RuntimeError('wrong infer_mode:{}, only support \"static\",\"dymbatch\",\"dymhw\", \
                \"dymdims\",\"dymshape\"'.format(mode))

        return self.run(inputs, out_array)

    def free_resource(self):
        if hasattr(self.session, "free_resource"):
            self.session.free_resource()

    def infer_pipeline(self, feeds_list, mode='static', custom_sizes=100000):
        '''
        Parameters:
            feeds_list: input data list
            mode: static dymdims dymshape...
        '''
        check_custom_size(custom_sizes, mode)
        inputs_list = []
        shapes_list = []
        for feeds in feeds_list:
            inputs = []
            shapes = []
            for feed in feeds:
                if type(feed) is np.ndarray:
                    infer_input = feed
                    if not infer_input.flags.c_contiguous:
                        infer_input = np.ascontiguousarray(infer_input)
                    shape = feed.shape
                elif type(feed) in NP_TYPE_LIST:
                    infer_input = np.array(feed)
                    if not infer_input.flags.c_contiguous:
                        infer_input = np.ascontiguousarray(infer_input)
                    shape = [feed.size]
                elif type(feed) is aclruntime.Tensor:
                    infer_input = np.array(feed)
                    shape = infer_input.shape
                elif hasattr(feed, 'type') and feed.type() in TORCH_TENSOR_LIST:
                    infer_input = feed.numpy()
                    infer_input = np.ascontiguousarray(infer_input) if not feed.is_contiguous() else infer_input
                    shape = infer_input.shape
                else:
                    raise RuntimeError('type:{} invalid'.format(type(feed)))
                basetensor = aclruntime.BaseTensor(infer_input.__array_interface__['data'][0], infer_input.nbytes)
                inputs.append(basetensor)
                shapes.append(shape)
            inputs_list.append(inputs)
            shapes_list.append(shapes)
        if self.infer_mode_switch.get(mode) is not None and mode != "dymshape" and mode != "dymdims":
            self.infer_mode_switch.get(mode)(shapes, custom_sizes)
        elif mode == "dymshape":
            if isinstance(custom_sizes, int):
                custom_sizes = [custom_sizes] * len(self.get_outputs())
            elif not isinstance(custom_sizes, list):
                raise RuntimeError('custom_sizes:{} type:{} invalid'.format(
                    custom_sizes, type(custom_sizes)))
            self.session.set_custom_outsize(custom_sizes)
        elif mode == "dymdims":
            pass
        else:
            raise RuntimeError('wrong infer_mode:{}, only support \"static\",\"dymbatch\",\"dymhw\", \
                \"dymdims\",\"dymshape\"'.format(mode))
        outputs = self.session.run_pipeline(self.outputs_names, inputs_list, shapes_list,
                                            mode == 'dymshape', mode == 'dymdims')
        for i, output in enumerate(outputs):
            outputs[i] = self.convert_tensors_to_arrays(output)
        return outputs

    def _create_device_inputs(self, feeds):
        inputs = []
        shapes = []
        for feed in feeds:
            if type(feed) is np.ndarray:
                infer_input = feed
                if not infer_input.flags.c_contiguous:
                    infer_input = np.ascontiguousarray(infer_input)
                shapes.append(infer_input.shape)
            elif type(feed) in NP_TYPE_LIST:
                infer_input = np.array(feed)
                if not infer_input.flags.c_contiguous:
                    infer_input = np.ascontiguousarray(infer_input)
                shapes.append([feed.size])
            elif hasattr(feed, 'type') and feed.type() in TORCH_TENSOR_LIST:
                infer_input = feed.numpy()
                if not feed.is_contiguous():
                    infer_input = np.ascontiguousarray(infer_input)
                shapes.append(infer_input.shape)
            else:
                raise RuntimeError('type:{} invalid'.format(type(feed)))
            acl_tensor = aclruntime.Tensor(infer_input)
            acl_tensor.to_device(self.device_id)
            inputs.append(acl_tensor)
        return inputs, shapes

    def _inner_iteration_run(self, inputs, in_out_list=None, iteration_times=1):
        out_names = [out_desc.name for out_desc in self.get_outputs()]
        outputs = self.session.run(out_names, inputs)
        if iteration_times == 1:
            return outputs
        for _ in range(int(iteration_times - 1)):
            for input_index, reused_index in enumerate(in_out_list):
                if reused_index >= 0:
                    inputs[input_index] = outputs[reused_index]
            outputs = self.session.run(out_names, inputs)

        return outputs

    def infer_iteration(self, feeds, in_out_list=None, iteration_times=1, mode='static',
            custom_sizes=100000):
        '''
        Parameters:
            feeds: input datas
            in_out_list: relation between current input datas and last output datas.
                [-1, 0, 1] means inputs[1] uses last outputs[0], inputs[2] uses last outputs[1].
            iteration_times: inner iteration infer loop times
            mode: static dymdims dymshape ...
            custom_sizes: only dymshape needs
        '''
        check_custom_size(custom_sizes, mode)
        check_positive_integer(iteration_times)
        if not in_out_list:
            in_out_list = []
        if in_out_list is not None:
            check_in_out_list(in_out_list, self.get_inputs(), self.get_outputs())

        inputs, shapes = self._create_device_inputs(feeds)

        # auto set mode
        if self.infer_mode_switch.get(mode) is not None:
            self.infer_mode_switch.get(mode)(shapes, custom_sizes)

        outputs = self._inner_iteration_run(inputs, in_out_list, iteration_times)

        self.convert_tensors_to_host(outputs)
        # convert tensor to narray
        return self.convert_tensors_to_arrays(outputs)


    def summary(self):
        return self.session.sumary()

    def _static_prepare(self, shapes, custom_sizes):
        self.set_staticbatch()

    def _dymbatch_prepare(self, shapes, custom_sizes):
        indesc = self.get_inputs()
        if (len(shapes) != len(indesc)):
            raise RuntimeError("input datas and intensors nums not matched!")
        for i, shape in enumerate(shapes):
            for j, dim in enumerate(shape):
                if (indesc[i].shape[j] < 0):
                    self.set_dynamic_batchsize(dim)
                    return
                if (indesc[i].shape[j] != dim):
                    raise RuntimeError("input datas and intensors dim not matched!")
        raise RuntimeError("not a dymbatch model!")

    def _dymhw_prepare(self, shapes, custom_sizes):
        indesc = self.get_inputs()
        if (len(shapes) != len(indesc)):
            raise RuntimeError("input datas and intensors nums not matched!")
        for i, shape in enumerate(shapes):
            if (indesc[i].shape[2] < 0 and indesc[i].shape[3] < 0):
                self.set_dynamic_hw(shape[2], shape[3])
                return
        raise RuntimeError("not a dymhw model!")

    def _dymdims_prepare(self, shapes, custom_sizes):
        dym_list = []
        indesc = self.get_inputs()
        if (len(shapes) != len(indesc)):
            raise RuntimeError("input datas and intensors nums not matched!")
        for i, shape in enumerate(shapes):
            str_shape = [str(val) for val in shape]
            dyshape = "{}:{}".format(indesc[i].name, ",".join(str_shape))
            dym_list.append(dyshape)
        dyshapes = ';'.join(dym_list)
        self.session.set_dynamic_dims(dyshapes)

    def _dymshape_prepare(self, shapes, custom_sizes):
        dym_list = []
        indesc = self.get_inputs()
        if (len(shapes) != len(indesc)):
            raise RuntimeError("input datas and intensors nums not matched!")
        outdesc = self.get_outputs()
        for i, shape in enumerate(shapes):
            str_shape = [str(val) for val in shape]
            dyshape = "{}:{}".format(indesc[i].name, ",".join(str_shape))
            dym_list.append(dyshape)
        dyshapes = ';'.join(dym_list)
        self.session.set_dynamic_shape(dyshapes)
        if isinstance(custom_sizes, int):
            custom_sizes = [custom_sizes] * len(outdesc)
        elif not isinstance(custom_sizes, list):
            raise RuntimeError('custom_sizes:{} type:{} invalid'.format(
                custom_sizes, type(custom_sizes)))
        self.session.set_custom_outsize(custom_sizes)


class MultiDeviceSession():
    def __init__(self, model_path: str, acl_json_path: str = None, debug: bool = False, loop: int = 1):
        check_model_path_legality(model_path)
        check_acl_json_path_legality(acl_json_path)
        check_loop_size(loop)
        self.model_path = model_path
        self.acl_json_path = acl_json_path
        self.debug = debug
        self.loop = loop
        self.summary = {}

    @classmethod
    def print_subprocess_run_error(cls, value):
        logger.error(f"subprocess run failed error_callback:{value}")

    def summary(self):
        return self.summary

    def infer(self, device_feeds:dict, mode='static', custom_sizes=100000):
        '''
        Parameters:
            device_feeds: device match [input datas1, input datas2...] (Dict)
        '''
        check_custom_size(custom_sizes, mode)
        subprocess_num = 0
        for _, device in device_feeds.items():
            subprocess_num += len(device)
        p = Pool(subprocess_num)
        outputs_queue = Manager().Queue()
        for device_id, feeds in device_feeds.items():
            check_device_range_valid(device_id)
            for feed in feeds:
                p.apply_async(
                    self.subprocess_infer,
                    args=(outputs_queue, device_id, feed, mode, custom_sizes),
                    error_callback=self.print_subprocess_run_error
                )
        p.close()
        p.join()
        result = 0 if 2 * len(device_feeds) == outputs_queue.qsize() else 1
        logger.info(f"multidevice run end qsize:{outputs_queue.qsize()} result:{result}")
        outputs_dict = {}
        self.summary.clear()
        while outputs_queue.qsize() != 0:
            ret = outputs_queue.get()
            if type(ret) == list:
                if (not outputs_dict.get(ret[0])):
                    outputs_dict.update({ret[0]: []})
                    self.summary.update({ret[0]: []})
                outputs_dict.get(ret[0]).append(ret[1])
                self.summary.get(ret[0]).append((ret[3] - ret[2]) * 1000)
                logger.info(f"device {ret[0]}, start_time:{ret[2]}, end_time:{ret[3]}")
        return outputs_dict

    def infer_pipeline(self, device_feeds_list:dict, mode='static', custom_sizes=100000):
        '''
        Parameters:
            device_feeds: device match [input datas1, input datas2...] (Dict)
        '''
        check_custom_size(custom_sizes, mode)
        subprocess_num = 0
        for _, device in device_feeds_list.items():
            subprocess_num += len(device)
        p = Pool(subprocess_num)
        outputs_queue = Manager().Queue()
        for device_id, feeds in device_feeds_list.items():
            check_device_range_valid(device_id)
            for feed in feeds:
                p.apply_async(
                    self.subprocess_infer_pipeline,
                    args=(outputs_queue, device_id, feed, mode, custom_sizes),
                    error_callback=self.print_subprocess_run_error
                )
        p.close()
        p.join()
        result = 0 if 2 * len(device_feeds_list) == outputs_queue.qsize() else 1
        logger.info(f"multidevice run pipeline end qsize:{outputs_queue.qsize()} result:{result}")
        outputs_dict = {}
        self.summary.clear()
        while outputs_queue.qsize() != 0:
            ret = outputs_queue.get()
            if type(ret) == list:
                if (not outputs_dict.get(ret[0])):
                    outputs_dict.update({ret[0]: []})
                    self.summary.update({ret[0]: []})
                outputs_dict.get(ret[0]).append(ret[1])
                self.summary.get(ret[0]).append((ret[3] - ret[2]) * 1000)
                logger.info(f"device {ret[0]}, start_time:{ret[2]}, end_time:{ret[3]}")
        return outputs_dict

    def infer_iteration(self, device_feeds:dict, in_out_list=None, iteration_times=1, mode='static', custom_sizes=None):
        '''
        Parameters:
            device_feeds: device match [input datas1, input datas2...] (Dict)
        '''
        check_custom_size(custom_sizes, mode)
        check_positive_integer(iteration_times)
        subprocess_num = 0
        for _, device in device_feeds.items():
            subprocess_num += len(device)
        p = Pool(subprocess_num)
        outputs_queue = Manager().Queue()
        for device_id, feeds in device_feeds.items():
            check_device_range_valid(device_id)
            for feed in feeds:
                p.apply_async(
                    self.subprocess_infer_iteration,
                    args=(outputs_queue, device_id, feed, in_out_list, iteration_times, mode, custom_sizes),
                    error_callback=self.print_subprocess_run_error
                )
        p.close()
        p.join()
        result = 0 if 2 * len(device_feeds) == outputs_queue.qsize() else 1
        logger.info(f"multidevice run iteration end qsize:{outputs_queue.qsize()} result:{result}")
        outputs_dict = {}
        self.summary.clear()
        while outputs_queue.qsize() != 0:
            ret = outputs_queue.get()
            if type(ret) == list:
                if (not outputs_dict.get(ret[0])):
                    outputs_dict.update({ret[0]: []})
                    self.summary.update({ret[0]: []})
                outputs_dict.get(ret[0]).append(ret[1])
                self.summary.get(ret[0]).append((ret[3] - ret[2]) * 1000)
                logger.info(f"device {ret[0]}, start_time:{ret[2]}, end_time:{ret[3]}")
        return outputs_dict

    def subprocess_infer(self, outputs_queue, device_id, feeds, mode='static', custom_sizes=100000):
        sub_session = InferSession(
            device_id=device_id,
            model_path=self.model_path,
            acl_json_path=self.acl_json_path,
            debug=self.debug,
            loop=self.loop
        )
        start_time = time.time()
        outputs = sub_session.infer(feeds, mode, custom_sizes, out_array=True)
        end_time = time.time()
        outputs_queue.put([device_id, outputs, start_time, end_time])
        return

    def subprocess_infer_pipeline(self, outputs_queue, device_id, feeds_list, mode='static', custom_sizes=100000):
        sub_session = InferSession(
            device_id=device_id,
            model_path=self.model_path,
            acl_json_path=self.acl_json_path,
            debug=self.debug,
            loop=self.loop
        )
        start_time = time.time()
        outputs = sub_session.infer_pipeline(feeds_list, mode, custom_sizes)
        end_time = time.time()
        outputs_queue.put([device_id, outputs, start_time, end_time])
        return

    def subprocess_infer_iteration(self, outputs_queue, device_id, feeds, in_out_list=None,
            iteration_times=1, mode='static', custom_sizes=None):
        sub_session = InferSession(
            device_id=device_id,
            model_path=self.model_path,
            acl_json_path=self.acl_json_path,
            debug=self.debug,
            loop=self.loop
        )
        if in_out_list is not None:
            check_in_out_list(in_out_list, sub_session.get_inputs(), sub_session.get_outputs())
        start_time = time.time()
        outputs = sub_session.infer_iteration(feeds, in_out_list, iteration_times, mode, custom_sizes)
        end_time = time.time()
        outputs_queue.put([device_id, outputs, start_time, end_time])
        return


class MemorySummary:
    @staticmethod
    def get_h2d_time_list():
        if hasattr(aclruntime, 'MemorySummary'):
            return aclruntime.MemorySummary().H2D_time_list
        else:
            return []

    @staticmethod
    def get_d2h_time_list():
        if hasattr(aclruntime, 'MemorySummary'):
            return aclruntime.MemorySummary().D2H_time_list
        else:
            return []

    @staticmethod
    def reset():
        if hasattr(aclruntime, 'MemorySummary'):
            aclruntime.MemorySummary().reset()
