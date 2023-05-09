#!/usr/bin/env python
# coding=utf-8
"""
Function:
AicoreErrorParser class. This file mainly involves the parse function.
Copyright Information:
Huawei Technologies Co., Ltd. All Rights Reserved © 2020
"""
import re
import os
import time
import json
import platform
import shutil
from functools import reduce
from ms_interface import utils
from ms_interface.constant import Constant
from ms_interface.aic_error_info import AicErrorInfo
from ms_interface.dump_data_parser import DumpDataParser
from ms_interface.single_op_case import SingleOpCase


class OpInputOutput:
    """
    Op input and output info
    """

    def __init__(self: any) -> None:
        self.name = ""  # input[0]
        self.idx = ""  # 0
        self.dtype = ""  # DT_FLOAT
        self.has_nan_inf = False
        self.size = 0  # in terms of bytes
        self.addr = ""
        self.actual_addr = ""
        self.overflow = False


class AicoreErrorParser:
    """
    The AicoreErrorParser class, parse aicore error info
    """

    def __init__(self: any, collection: any, output_path: str, collect_time: any) -> None:
        self.collection = collection
        self.output_path = output_path
        self.collect_time = collect_time

    def _get_graph_file(self: any) -> str:
        utils.print_info_log("Start to get graph file.")
        match_list = []
        for top, _, files in os.walk(os.path.join(self.output_path, 'collection', 'compile')):
            for name in files:
                file_name_pattern = re.compile(Constant.BUILD_PROTO_FILE_PATTERN)
                pattern_match = file_name_pattern.match(name)
                if pattern_match:
                    match_list.append((pattern_match.group(1), os.path.join(top, name)))
        new_match_list = sorted(match_list, key=lambda s: s[0], reverse=True)
        choose_file = new_match_list[0][1]
        utils.print_info_log(f'Choose {choose_file} to read op info.')
        return choose_file

    @staticmethod
    def _get_op_by_graph(graph_file: str, info):
        try:
            with open(graph_file, 'r') as graph:
                text = graph.read()
                regexp = r'(op\s+\{\s+name:\s+"%s".+?%s.+?\})\s+op\s+\{' % (info.node_name, info.kernel_name)
                ret = re.findall(regexp, text, re.M | re.S)
                if len(ret) == 0:
                    utils.print_warn_log(f'Failed to get op for node({info.node_name}) kernel({info.kernel_name}).')
                    return
                info.operator = ret[0]
        except IOError as io_error:
            utils.print_error_log(f'Failed to open file {graph_file}. {io_error}')
            raise utils.AicErrException(Constant.MS_AICERR_OPEN_FILE_ERROR)

    def _write_summary_file(self: any, summary_info_list: list) -> None:
        summary = """本次信息收集发生于%s，共收集到%d个AICERROR，概要如下：
***************************************************************************************************
%s
***************************************************************************************************
建议选择最近发生的AICERROR，查看其中的info.txt

""" % (time.strftime("%Y-%m-%d %H:%M:%S", self.collect_time),
       len(self.collection.ai_core_error_list), "\n".join(summary_info_list))
        summary_file = os.path.join(self.output_path, "README.txt")
        utils.write_file(summary_file, summary)
        utils.print_info_log('The summary info is saved in %s' % summary_file)

        utils.print_info_log('Analysis finished, please check %s, you can '
                             'view README.txt first.' % self.output_path)

    def _get_atomic_err_log(self: any) -> list:
        cmd = ['grep', 'dha status 1', '-nr', self.collection.collect_plog_path]
        status, _ = utils.execute_command(cmd)
        if status == 0:
            utils.print_info_log("#" * 70)
            utils.print_info_log("Find \"dha status 1\" in plogs. Maybe atomic add error happened!")
            utils.print_info_log("#" * 70)
            return True
        return False

    def _get_alloc_addr(self: any) -> list:
        cmd = ['grep', 'DevMalloc: Succ,', '-nr', self.collection.collect_plog_path]
        regexp = r"(\d+-\d+-\d+-\d+:\d+:\d+\.\d+\.\d+).+?size\s*=\s*([\d]+).+?ptr\s*=\s*([\da-zA-Z]+)"
        ret = utils.get_inquire_result(cmd, regexp)
        alloc_addr = []
        for _, (_, size, addr) in enumerate(ret):
            alloc_addr.append((addr, int(size)))
        return alloc_addr

    @staticmethod
    def _remove_first_found_addr(addr, addr_list):
        for i, ava_addr_item in enumerate(addr_list):
            if addr == ava_addr_item[0]:
                addr_list.pop(i)
                break
        return addr_list

    def _get_available_addrs(self: any, occur_time: str) -> list:
        '''
        获取occur_time时刻可用的地址
        :param occur_time: aicore error发生的时间
        :return: 可用空间的list
        '''
        alloc_cmd = ['grep', 'DevMalloc: Succ,', '-nr', self.collection.collect_plog_path]
        alloc_regexp = r"(\d+-\d+-\d+-\d+:\d+:\d+\.\d+\.\d+).+?size\s*=\s*([\d]+).+?ptr\s*=\s*([\da-zA-Z]+)"
        alloc_ret = utils.get_inquire_result(alloc_cmd, alloc_regexp)

        free_cmd = ['grep', 'DevFree: mem', '-nr', self.collection.collect_plog_path]
        free_regexp = r"(\d+-\d+-\d+-\d+:\d+:\d+\.\d+\.\d+).+?mem\s*=\s*([\da-zA-Z]+)"
        free_ret = utils.get_inquire_result(free_cmd, free_regexp)
        avl_addr = []
        occur_time_obj = utils.strplogtime(occur_time)
        for _, (alloc_time, size, addr) in enumerate(alloc_ret):
            alloc_time_obj = utils.strplogtime(alloc_time)

            if alloc_time_obj < occur_time_obj:
                avl_addr.append((addr, int(size)))

        for _, (free_time, addr) in enumerate(free_ret):
            free_time_obj = utils.strplogtime(free_time)
            if free_time_obj < occur_time_obj:
                avl_addr = self._remove_first_found_addr(addr, avl_addr)
        utils.print_info_log(f"get available addr: {avl_addr}")
        return avl_addr

    def _get_necessary_addrs(self: any, kernel_name: str) -> dict:
        '''
        获取occur_time时刻可用的地址
        :param kernel_name: 发生aicore error的kernel_name
        :return: 需要的空间
        '''
        result = {}
        aic_info_cmd = ['grep', '-r', '-C', '7', "\[AIC_INFO\] dev_func:{}".format(kernel_name),
                        self.collection.collect_plog_path]
        _, aic_info = utils.execute_command(aic_info_cmd)
        utils.print_info_log(f"===============================\n{aic_info}\n==================================")

        # 解析出输入参数
        aic_info_input_regexp = r"\[AIC_INFO\]\sinput:(.*?);shape:(.*?);format:(.*?);dtype:(.*?);addr:(.*?)$"
        aic_info_input_ret = re.findall(aic_info_input_regexp, aic_info, re.M)
        if len(aic_info_input_ret) == 0:
            utils.print_warn_log(f"Failed to get {aic_info_input_regexp}")
            return result
        input_params = []

        for input_info in aic_info_input_ret:
            input_param = {}
            input_param["index"] = input_info[0]
            input_param["shape"] = input_info[1]
            input_param["format"] = input_info[2]
            input_param["dtype"] = input_info[3]
            input_param["addr"] = input_info[4]
            input_params.append(input_param)

        # 解析出输出结果
        aic_info_output_regexp = r"\[AIC_INFO\]\soutput:(.*?);shape:(.*?);format:(.*?);dtype:(.*?);addr:(.*?)$"
        aic_info_output_ret = re.findall(aic_info_output_regexp, aic_info, re.M)
        if len(aic_info_output_ret) == 0:
            utils.print_warn_log(f"Failed to get {aic_info_output_regexp}")
            return result
        output_params = []
        for output_info in aic_info_output_ret:
            output_param = {}
            output_param["index"] = output_info[0]
            output_param["shape"] = output_info[1]
            output_param["format"] = output_info[2]
            output_param["dtype"] = output_info[3]
            output_param["addr"] = output_info[4]
            output_params.append(output_param)

        aic_info_workspace_regex = r"\[AIC_INFO\]\sworkspace_bytes|workspace_size:(.*?)"
        aic_info_workspace_ret = re.findall(aic_info_workspace_regex, aic_info, re.M)
        if len(aic_info_workspace_ret) == 0:
            utils.print_warn_log(f"Failed to get {aic_info_workspace_regex} from [AIC_INFO].")
        elif len(aic_info_workspace_ret[0]) == 0:
            utils.print_warn_log(f"Failed to get {aic_info_workspace_regex}.")
            workspace = "0"
        else:
            workspace = aic_info_workspace_ret[0][0]

        # tiling
        tiling_data_regexp = r"\[AIC_INFO\]\stiling_data:(.*)"
        tiling_data_ret = re.findall(tiling_data_regexp, aic_info, re.M)
        if len(tiling_data_ret) == 0:
            utils.print_warn_log(f"Failed to get {tiling_data_regexp}")
            tiling_data = None
        else:
            if tiling_data_ret[0].startswith("0x"):
                temp_tiling_data = tiling_data_ret[0].replace(" 0x", "")[2:-1]
                tiling_data = bytes.fromhex(temp_tiling_data)
            else:
                tiling_data = bytes(tiling_data_ret[0], 'utf-8')

        tiling_key_regexp = r"\[AIC_INFO\]\stiling_key:([0-9]{1,})"
        tiling_key_ret = re.findall(tiling_key_regexp, aic_info, re.M)
        if len(tiling_key_ret) == 0:
            utils.print_warn_log(f"Failed to get {tiling_key_regexp}")
            tiling_key = "0"
        else:
            tiling_key = tiling_key_ret[0]

        block_dim_regexp = r"\[AIC_INFO\]\sblock_dim:(\d+)"
        block_dim_ret = re.findall(block_dim_regexp, aic_info, re.M)
        if len(block_dim_ret) == 0:
            utils.print_warn_log(f"Failed to get {block_dim_regexp} from [AIC_INFO].")
        elif len(block_dim_ret[0]) == 0:
            utils.print_warn_log(f"Failed to get {block_dim_regexp} is null.")
            block_dim = -1
        else:
            block_dim = int(block_dim_ret[0])

        result["input_addr"] = input_params
        result["output_addr"] = output_params
        result["workspace"] = workspace
        result["tiling"] = [tiling_key, tiling_data, block_dim]
        return result

    @staticmethod
    def _cal_shape_size(shape_str):
        utils.print_info_log("shape_str is {}".format(shape_str))
        if shape_str == "[]":
            return 1
        shape_str_list = shape_str.replace("[", "").replace("]", "").split(",")
        return reduce(lambda x, y: int(x) * int(y), shape_str_list)
    
    @staticmethod
    def _check_addr_in_range(addr, ranges):
        if not isinstance(addr, int):
            addr = int(addr)

        for addr_range in ranges:
            range_left = utils.get_hexstr_value(addr_range[0])
            range_right = utils.get_hexstr_value(addr_range[0]) + addr_range[1]
            if range_left <= addr < range_right:
                return True
        return False

    def _check_addr(self, avaliable_addrs, used_addrs):
        input_params = used_addrs.get("input_addr")
        output_params = used_addrs.get("output_addr")
        if not input_params and not output_params:
            utils.print_error_log("Unable to get input parameters and output parameters.")
            raise utils.AicErrException(Constant.MS_AICERR_FIND_DATA_ERROR)

        for input_param in input_params:
            if input_param.get("addr").startswith("0x"):
                start_addr = int(input_param.get("addr"), 16)
            else:
                start_addr = int(input_param.get("addr"))
            shape_size = self._cal_shape_size(input_param.get("shape"))
            size_of_dtype = Constant.SIZE_OF_DTYPE.get(input_param.get("dtype"))
            end_addr = int(start_addr) + int(shape_size) * int(size_of_dtype)
            utils.print_info_log(f"shape_size is {shape_size}, size_of_dtype is {size_of_dtype}")
            input_param["size"] = int(shape_size) * int(size_of_dtype)

        for output_param in output_params:
            if output_param.get("addr").startswith("0x"):
                start_addr = int(output_param.get("addr"), 16)
            else:
                start_addr = int(output_param.get("addr"))
            shape_size = self._cal_shape_size(output_param.get("shape"))
            size_of_dtype = Constant.SIZE_OF_DTYPE.get(output_param.get("dtype"))
            end_addr = int(start_addr) + int(shape_size) * int(size_of_dtype)
            utils.print_info_log(f"shape_size is {shape_size}, size_of_dtype is {size_of_dtype}")
            output_param["size"] = int(shape_size) * int(size_of_dtype)

    def _get_input_output_addrs(self: any, info: any, alloc_addr: str) -> list:
        in_out_list = info.necessary_addr
        input_output_addrs = []
        for input in in_out_list.get("input_addr"):
            op_io = OpInputOutput()
            op_io.name = f"input[{input.get('index')}]"
            op_io.addr = input.get('addr')
            op_io.idx = int(input.get('index'))
            op_io.dtype = input.get('dtype')
            op_io.size = input.get('size')
            input_output_addrs.append(op_io)
        for output in in_out_list.get("output_addr"):
            op_io = OpInputOutput()
            op_io.name = f"output[{output.get('index')}]"
            op_io.addr = output.get('addr')
            op_io.idx = int(output.get('index'))
            op_io.dtype = output.get('dtype')
            op_io.size = output.get('size')
            input_output_addrs.append(op_io)

        # 检查地址是否越界
        if len(alloc_addr) > 0:
            self._analyse_alloc_addr_range(alloc_addr, input_output_addrs)
        return input_output_addrs

    @staticmethod
    def _get_alloc_addr_range(alloc_addr: any) -> list:
        alloc_addr_range = []
        for alloc in alloc_addr:
            begin_alloc = int(alloc[0], 16)
            end_alloc = begin_alloc + alloc[1] - 1 if alloc[1] > 0 else begin_alloc
            alloc_addr_range.append((begin_alloc, end_alloc))
        return alloc_addr_range

    def _analyse_alloc_addr_range(self: any, alloc_addr: any, input_output_addrs: list) -> None:
        alloc_addr_range = self._get_alloc_addr_range(alloc_addr)
        for i in input_output_addrs:
            begin_addr = i.addr
            end_addr = begin_addr + i.size - 1 if i.size > 0 else begin_addr
            begin_in_range = self._check_addr_in_alloc(alloc_addr_range, begin_addr)
            if begin_in_range is False:
                i.overflow = True
                continue
            end_in_range = self._check_addr_in_alloc(alloc_addr_range, end_addr)
            if end_in_range is False:
                i.overflow = True
            # check actual addr
            self._check_actual_addr(i, alloc_addr_range)

    @staticmethod
    def _check_addr_in_alloc(alloc_addr_range: list, alloc_addr: int) -> bool:
        alloc_in_range = False
        for _, (begin_alloc, end_alloc) in enumerate(alloc_addr_range):
            if begin_alloc <= alloc_addr <= end_alloc:
                alloc_in_range = True
                break
        return alloc_in_range

    @staticmethod
    def _check_actual_addr(i: any, alloc_addr_range: list) -> None:
        # check actual addr
        if i.actual_addr != "":
            actual_addr = int(i.actual_addr, 16)
            actual_in_range = False
            for _, (begin_alloc, end_alloc) in enumerate(
                    alloc_addr_range):
                if begin_alloc <= actual_addr <= end_alloc:
                    actual_in_range = True
                    break
            if actual_in_range is False:
                i.overflow = True

    def _get_info_for_decompile(self: any, info: any) -> tuple:
        info.instr = ""
        # 最后一条指令 算差值
        utils.print_info_log("Calculate the address offset.")
        current_pc5 = '0' + info.current_pc[-5:]
        start_pc5 = '0' + info.start_pc[-5:]
        current_pc5_value = utils.get_hexstr_value(current_pc5)
        start_pc5_value = utils.get_hexstr_value(start_pc5)
        diff_str = hex(current_pc5_value - start_pc5_value)[2:]
        utils.print_info_log(f"The address offset is {diff_str}.")

        # 估算err pc
        utils.print_info_log("Computes the estimated error address offset.")
        err_pc = self._get_err_pc(info, current_pc5_value, start_pc5_value)
        if err_pc == "":
            utils.print_info_log("This aicore error has no estimated offset.")
        else:
            utils.print_info_log(f"Estimated error address offset is {err_pc}.")

        return diff_str, err_pc

    @staticmethod
    def _get_decompile_status(o_file: str, decompile_file: str) -> int:
        cmd = [Constant.OBJ_DUMP_FILE, '-d', '--mcpu=dav-m100', '--line-numbers', o_file]
        status, _ = utils.execute_command(cmd, file_out=decompile_file)
        return status

    def _decompile(self: any, kernel_meta_path: str, dir_path: str, info: any) -> bool:
        kernel_name = info.kernel_name
        diff_str, err_pc = self._get_info_for_decompile(info)
        tiling_key = info.necessary_addr["tiling"][0]

        # decompile .o file
        cce_file = os.path.join(kernel_meta_path, kernel_name + ".cce")
        if not os.path.exists(cce_file):
            utils.print_warn_log(f"The cce file:{cce_file} does not exist.")
            cce_file = os.path.join(kernel_meta_path, f"{kernel_name}_{tiling_key}.cce")
            if not os.path.exists(cce_file):
                utils.print_error_log(f"The cce file:{cce_file} does not exist.")
                return False
        o_file = os.path.join(kernel_meta_path, kernel_name + ".o")
        json_file = os.path.join(kernel_meta_path, kernel_name + ".json")
        if not os.path.exists(o_file) and not os.path.exists(json_file):
            utils.print_error_log("The file that needs to be decompiled does not exist.")
            return False

        # decompile .o file
        decompile_file = os.path.join(dir_path, kernel_name + ".o.txt")
        status = self._get_decompile_status(o_file, decompile_file)
        if status != 0:
            utils.print_error_log(f"Failed to decompile {o_file}, you can fix problem according to the message above, "
                                  f"or copy {Constant.OBJ_DUMP_FILE} and {o_file} to another host and execute : "
                                  f"{Constant.OBJ_DUMP_FILE} -d -mcpu=dav-m100 {kernel_name}.o > {kernel_name}.o.txt")
            return False
        utils.copy_src_to_dest([cce_file, o_file, json_file], dir_path)
        loc_json_file = os.path.join(kernel_meta_path, kernel_name + "_loc.json")
        cce_tbe_result = self._get_cce_tbe_code_number(decompile_file, loc_json_file, err_pc, info)
        occur_result = self._get_occur_before_mark(decompile_file, diff_str, info)

        return cce_tbe_result and occur_result

    @staticmethod
    def _get_err_pc(info: any, current_pc5_value: int, start_pc5_value: int) -> str:
        # 估算err pc
        extra_pc = info.find_extra_pc()  # [9:2]
        err_pc = ""
        if extra_pc != "":
            ori_pc = bin(current_pc5_value)[2:]
            if len(ori_pc) == 1:
                new_pc_bin = extra_pc + '0' + ori_pc
            elif len(ori_pc) <= 10:
                new_pc_bin = extra_pc + ori_pc[-2:]
            else:
                new_pc_bin = ori_pc[:-10] + extra_pc + ori_pc[-2:]
            new_pc_value = int(new_pc_bin, 2)
            if new_pc_value > current_pc5_value:
                new_pc_value -= 1024
            err_pc = hex(new_pc_value - start_pc5_value)[2:]
            info.instr += "\nError occured most likely at line: %s\n\n" % err_pc

        return err_pc

    @staticmethod
    def _read_decompile_file(decompile_file: str, err_pc: str, info: any) -> str:
        with open(decompile_file, 'r') as fo_file:
            cce_code = ""
            cce_code_num = ""
            for line in fo_file.readlines():
                regexp = r"cce:(\d+)"
                ret = re.findall(regexp, line, re.M)
                if len(ret) > 0:
                    cce_code = line
                    cce_code_num = ret[0]
                elif err_pc + ':' in line:
                    info.instr += "%s:%s\n" % (fo_file.name, err_pc)
                    break
            info.instr += "%s" % cce_code.split(os.sep)[-1]
        return cce_code_num

    @staticmethod
    def _read_loc_json_file(loc_json_file: str, cce_code_num: str, info: any) -> None:
        with open(loc_json_file, 'r') as load_f:
            load_dict = json.load(load_f)
            for line in load_dict[0]['cce_line2loc']:
                if str(line['cce_line']) == cce_code_num \
                        and line['loc'][0] != "":
                    info.instr += "%s:%s\n" % (line['loc'][0], line['loc'][1])

    def _get_cce_tbe_code_number(self: any, decompile_file: str, loc_json_file: str, err_pc: str, info: any) -> bool:
        # txt code to cce number
        if os.path.exists(decompile_file) is False:
            utils.print_error_log("The decompile file does not exist.")
            return False

        if err_pc != "":
            cce_code_num = self._read_decompile_file(decompile_file, err_pc, info)
            # cce to tbe code number
            if not os.path.exists(loc_json_file) or os.stat(loc_json_file).st_size == 0:
                utils.print_warn_log(f"The file {loc_json_file} is not exist or file is empty.")
                return True
            self._read_loc_json_file(loc_json_file, cce_code_num, info)
        return True

    @staticmethod
    def __generate_case(config_file):
        dir_name = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        case_content = f"from ms_interface.single_op_case import SingleOpCase\nSingleOpCase.run(\"{config_file}\")"
        case_file = os.path.join(dir_name, "test_single_op.py")
        utils.print_info_log(f"Generate case file {case_file}")
        with open(case_file, 'w') as f:
            f.write(case_content)
        return case_file
             
    @staticmethod
    def _test_single_op(collection):
        single_op_case = SingleOpCase(collection)
        config_list = single_op_case.generate_config()
        for config in config_list:
            single_op_ret = single_op_case.run(config)
            if not single_op_ret:
                case_file = AicoreErrorParser.__generate_case(config)
                split_line = "#" * 50
                utils.print_info_log(split_line)
                utils.print_info_log("single op test failed! Please Check OP or input data!")
                utils.print_info_log(f"Run 'python3 {case_file}' can test op!")
                utils.print_info_log(split_line)
                return False
        return True

    @staticmethod
    def _get_occur_before_mark(decompile_file: str, diff_str: str, info: any) -> bool:
        #      504:    04c20000    ST.b64         X1, [X0], #0
        with open(decompile_file, "r") as fo_file:
            text = fo_file.read()

        regexp = r'(^\s+(\S+):\s+\S+\s+\S.+$)'
        ret = re.findall(regexp, text, re.M)
        find_i = -1
        for i, (_, line_diff) in enumerate(ret):
            if line_diff == diff_str:
                find_i = i
                break

        if find_i == -1:
            utils.print_warn_log(f"Get fault instruction failed, file({decompile_file}) diff({diff_str}).")
            return False

        begin_i = 0 if find_i < 9 else find_i - 9
        instr_str_list = []
        for i in range(begin_i, find_i + 1):
            instr_str_list.append(ret[i][0] + "\n")
        instr_str = "".join(instr_str_list).strip("\n")

        info.instr += "\nrelated instructions (error occured before the mark *):\n\n"
        info.instr += instr_str[:instr_str.rfind('\n') + 1] + '*'
        info.instr += instr_str[instr_str.rfind('\n') + 2:]
        info.instr += "\n\nFor complete instructions, please view %s" % decompile_file

        return True

    @staticmethod
    def _write_errorinfo_file(err_i_folder: str, info: AicErrorInfo, index: int) -> None:
        info_file = os.path.join(err_i_folder, "info.txt")
        utils.write_file(info_file, info.analyse())
        utils.print_info_log(f'The ai core error info for No.{index} is saved in {info_file}')

    def _aicore_error_data(self: any) -> list:
        utils.print_info_log("Start to get DevMalloc address information.")
        aicore_error_data_list = []

        # 获取分配的device地址
        alloc_addr = []  # self._get_alloc_addr()
        utils.print_warn_log("Due to security issues, DevMalloc address information cannot be obtained.")

        # get graph file
        graph_file = self._get_graph_file()
        aicore_error_data_list.extend([alloc_addr, graph_file])
        return aicore_error_data_list

    def parse(self: any) -> None:
        """
        parse by collection info
        """
        utils.print_info_log('******************Analysis******************')
        aicore_error_data_list = self._aicore_error_data()
        summary_info_list = []

        # decompile
        utils.print_info_log("Start looking for the location of cce-objdump.")
        obj_dump_file = "cce-objdump_aarch64" if "aarch64" in platform.machine() else "cce-objdump"
        obj_dump_file = os.path.join(os.getcwd(), "tools", obj_dump_file)
        if os.path.exists(obj_dump_file):
            os.system("chmod 755 " + obj_dump_file)
            os.environ["PATH"] = os.path.join(os.getcwd(), "tools") + ":" + os.environ["PATH"]
        else:
            cce_dump = shutil.which("cce-objdump")
            if not cce_dump:
                # guess where is cce-objdump
                parent_path = "aarch64-linux" if "aarch64" in platform.machine() else "x86_64-linux"
                cce_dump_guess = os.path.join("/usr/local/Ascend/latest", parent_path, "ccec_compiler/bin/cce-objdump")
                if os.path.exists(cce_dump_guess):
                    cce_dump = cce_dump_guess
                else:
                    utils.print_error_log("Cannot find cce-objdump! please add cce-objdump path in env PATH.")
                    raise utils.AicErrException(Constant.MS_AICERR_EXECUTE_COMMAND_ERROR)
        for i, current_pc in enumerate(self.collection.ai_core_error_list):
            # parser aic error by slog
            info = AicErrorInfo()
            info.err_time, info.dev_id, info.stream_id, info.task_id, info.core_id, info.aic_error, info.start_pc, \
                info.extra_info, info.current_pc = current_pc
            info.node_name = self.collection.node_name_list[i]
            info.kernel_name = self.collection.kernel_name_list[i]
            
            if "0x800000" == info.aic_error:
                info.atomic_add_err = self._get_atomic_err_log()

            utils.print_info_log(f"******************No.{i} {info.err_time}******************")
            info.err_time_obj = utils.strplogtime(info.err_time)
            err_i_folder_name = f"aicerror_{i}_{time.strftime('%Y%m%d%H%M%S', info.err_time_obj.timetuple())}"
            err_i_folder = os.path.join(self.output_path, err_i_folder_name)
            utils.check_path_valid(err_i_folder, isdir=True, output=True)
            # get op info in build proto file
            self._get_op_by_graph(aicore_error_data_list[Constant.GRAPH_FILE], info)

            try:
                # input output address
                info.aval_addrs = []  # self._get_available_addrs(info.err_time)
                info.necessary_addr = self._get_necessary_addrs(info.kernel_name)
                self.collection.tiling_list = info.necessary_addr["tiling"]
                # 校验地址
                self._check_addr(info.aval_addrs, info.necessary_addr)
            except Exception as e:
                utils.print_error_log("Check addr error failed.")
                raise utils.AicErrException(Constant.MS_AICERR_FIND_DATA_ERROR) from e

            kernel_meta_path = self.collection.collect_kernel_path
            # 反编译  出错指令
            result = self._decompile(kernel_meta_path, err_i_folder, info)
            if not result:
                utils.print_warn_log(f"decompile kernel_meta file \
                    {os.path.join(kernel_meta_path, info.kernel_name)}.o failed.")

            info.input_output_addrs = self._get_input_output_addrs(info, aicore_error_data_list[Constant.ALLOC_ADDR])

            # parse dump
            if self.collection.collect_dump_path:
                dump_parser = DumpDataParser(self.collection.collect_dump_path, info.node_name, info.kernel_name)
                info.dump_info = dump_parser.parse()
                self.collection.input_list = dump_parser.get_input_data()
                self.collection.output_list = dump_parser.get_output_data()
            info.single_op_test_result = self._test_single_op(self.collection)
            # write info file
            self._write_errorinfo_file(err_i_folder, info, i)

            summary_info_list.append(
                "%s   %s   device_id=%s   core_id=%s   task_id=%s   node=%s   "
                "kernel=%s" % (err_i_folder_name, info.aic_error, info.dev_id,
                               info.core_id, info.task_id, info.node_name,
                               info.kernel_name))
        utils.print_info_log('Finish to analyze each ai core error.')
        # write summary info
        self._write_summary_file(summary_info_list)
