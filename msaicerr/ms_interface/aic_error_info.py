#!/usr/bin/env python
# coding=utf-8
"""
Function:
AicoreErrorParser class. This file mainly involves the parse function.
Copyright Information:
Huawei Technologies Co., Ltd. All Rights Reserved © 2020
"""

import re
from ms_interface import utils
from ms_interface.constant import Constant


class AicErrorInfo:
    """
    AI core Error info
    """
    def __init__(self: any) -> None:
        self.dev_id = ""
        self.core_id = ""
        self.aic_error = ""  # err_code
        self.aicerror_bit = []  # [1,2,3...]
        self.task_id = ""
        self.stream_id = ""
        self.node_name = ""
        self.kernel_name = ""
        self.start_pc = ""
        self.current_pc = ""
        self.input_output_addrs = []  # [OpInputOutput]
        self.instr = ""
        self.operator = ""
        self.extra_info = ""
        self.err_time_obj = None
        self.err_time = ""
        self.ifu_err_type = ""
        self.mte_err_type = ""
        self.args_num_in_json = None
        self.dump_info = ""
        self.aval_addrs = []
        self.necessary_addr = {}

        self.ifu_key = "IFU_ERR_INFO"
        self.ccu_key = "CCU_ERR_INFO"
        self.biu_key = "BIU_ERR_INFO"
        self.cube_key = "CUBE_ERR_INFO"
        self.mte_key = "MTE_ERR_INFO"
        self.vec_key = "VEC_ERR_INFO"

    def analyse(self: any) -> str:
        """
        AI core error analyse
        """
        # 此步骤会解析出 mte_err_type ifu_err_type
        aicerror_info = self._get_aicerror_info()

        addr_check_str = self._get_addr_check_str()

        msg = """
***********************1. Basic information********************
error time   : %s
device id    : %s
core id      : %s
task id      : %s
stream id    : %s
node name    : %s
kernel name  : %s

***********************2. AICERROR code***********************
error code  : %s
error bits : 
%s

***********************3. Instructions************************
start   pc   : %s
current pc   : %s
%s

****************4. Input and output of node*******************
%s

***********************5. Op in graph*************************
%s

""" % (self.err_time, self.dev_id, self.core_id, self.task_id, 
       self.stream_id, self.node_name, self.kernel_name,
       self.aic_error, aicerror_info, self.start_pc, 
       self.current_pc, self.instr, addr_check_str, self.operator)
        if self.dump_info != "":
            msg += """
***********************6. Dump info*************************
%s
""" % self.dump_info

        # 0x800000 推断
        conclusion = ""
        if self.current_pc == "0x0":
            conclusion = "Memory of operator code has been overwrited falsely\n"
        if conclusion != "":
            msg = "********************Root cause conclusion****************" \
                  "*****\n%s\n" % conclusion + msg
        else:
            msg = "********************Root cause conclusion****************" \
                  "*****\n%s\n" % "Not available" + msg
        msg = "Analysis result: success.\n" + msg
        return msg

    def _get_addr_check_str(self: any) -> str:
        result_str = ""
        used_addrs = self.necessary_addr
        ava_addr = self.aval_addrs
        if not used_addrs:
            input_params, output_params = [], []
        else:
            input_params = used_addrs.get("input_addr")
            output_params = used_addrs.get("output_addr")
        workspace = used_addrs.get("workspace")
        for input_param in input_params:
            index = input_param.get("index")
            if input_param.get("invalid"):
                result_str += f"*[ERROR]input[{index}] is out of range\n"

        for output_param in output_params:
            index = output_param.get("index")
            if output_param.get("invalid"):
                result_str += f"*[ERROR]output[{index}] is out of range\n"
        result_str += "\n"
        for input_param in input_params:
            index = int(input_param.get("index"))
            size = int(input_param.get("size"))
            addr = int(input_param.get("addr"), 16) if input_param.get("addr").startswith("0x") else int(input_param.get("addr"))
            end_addr = addr + size
            result_str += f"input[{index}] addr: {hex(addr)} end_addr:{hex(end_addr)} size: {hex(size)}\n"

        for output_param in output_params:
            index = int(output_param.get("index"))
            size = int(output_param.get("size"))
            addr = int(output_param.get("addr"), 16) if output_param.get("addr").startswith("0x") else int(output_param.get("addr"))
            end_addr = addr + size
            result_str += f"output[{index}] addr: {hex(addr)} end_addr:{hex(end_addr)} size: {hex(size)}\n"

        if workspace:
            result_str += f"workspace_bytes:{workspace}\n"
            
        result_str += "\n\nDue to security issues, DevMalloc address information cannot be obtained."
      
        return result_str

    def _get_aicerror_info(self: any) -> str:
        aicerror_info_list = []
        handled_err_type = []
        for i in self.aicerror_bit:
            aicerr_info = Constant.AIC_ERROR_INFO_DICT.get(i)
            err_type = aicerr_info.split('_')[0].lower()
            if err_type in handled_err_type:
                continue
            handled_err_type.append(err_type)
            if err_type == "vec":
                aicerror_info_list.append("\nVEC_ERR_INFO: " + self._analyse_vec_errinfo())
            elif err_type == "ifu":
                aicerror_info_list.append("\nIFU_ERR_INFO: " + self._analyse_ifu_errinfo())
            elif err_type == "mte":
                aicerror_info_list.append("\nMTE_ERR_INFO: " + self._analyse_mte_errinfo(i))
            elif err_type == "cube":
                aicerror_info_list.append("\nCUBE_ERR_INFO: " + self._analyse_cube_errinfo())
            elif err_type == "ccu":
                aicerror_info_list.append("\nCCU_ERR_INFO: " + self._analyse_ccu_errinfo())
            elif err_type == "biu":
                aicerror_info_list.append("\nBIU_ERR_INFO: " + self._analyse_biu_errinfo())
            aicerror_info_list.append(f"\n{aicerr_info}")
            aicerror_info_list.append("\n\n")
        aicerror_info = "".join(aicerror_info_list).strip("\n")
        return aicerror_info

    # Error PC [9:2]
    def find_extra_pc(self: any) -> str:
        """
        find extra pc
        """
        ret = utils.hexstr_to_list_bin(self.aic_error)
        if not ret:
            ret = [0]
        self.aicerror_bit = ret
        extra_err_key = ""
        key_map = {"vec": self.vec_key,
                   "mte": self.mte_key,
                   "cube": self.cube_key,
                   "ccu": self.ccu_key,
                   "biu": self.biu_key,
                   "ifu": self.ifu_key}
        for ret_a in ret:
            error_info = Constant.AIC_ERROR_INFO_DICT.get(ret_a)
            err_type = error_info.split('_')[0].lower()
            if err_type in key_map.keys():
                extra_err_key = key_map.get(err_type)
                break

        if extra_err_key == "":
            return ""
        regexp = extra_err_key + r"=(\S+)"
        ret = re.findall(regexp, self.extra_info, re.M)
        if len(ret) == 0:
            return ""
        return utils.get_01_from_hexstr(ret[0], 7, 0)

    def _analyse_ifu_errinfo(self: any) -> str:
        regexp = self.ifu_key + r"=(\S+)"
        ret = re.findall(regexp, self.extra_info, re.M)
        if len(ret) == 0:
            return "No IFU_ERR_INFO found"

        errinfo = ret[0]
        # ifu_err_type
        code = utils.get_01_from_hexstr(ret[0], 50, 48)
        self.ifu_err_type = code

        if code in Constant.SOC_ERR_INFO_DICT:
            info = Constant.SOC_ERR_INFO_DICT.get(code)
        else:
            info = "NA"
        errinfo += f"\nifu_err_type bit[50:48]={code}  meaning:{info}"

        # ifu_err_addr
        code = utils.get_01_from_hexstr(ret[0], 47, 2)
        info = "IFU Error Address [47:2]"
        # 补2位0，猜测值
        approximate = hex(int(code + "00", 2))
        errinfo += f"\nifu_err_addr bit[47:2]={code}  meaning:{info}  approximate:{approximate}"
        return errinfo

    def _analyse_mte_errinfo(self: any, err_bit: any) -> str:
        regexp = self.mte_key + r"=(\S+)"
        ret = re.findall(regexp, self.extra_info, re.M)
        if len(ret) == 0:
            return "No MTE_ERR_INFO found"

        errinfo = ret[0]
        # mte_err_type
        code = utils.get_01_from_hexstr(ret[0], 26, 24)
        self.mte_err_type = code

        if err_bit == 46:
            mte_dict = Constant.UNZIP_ERR_INFO_DICT
        elif err_bit == 34:
            mte_dict = Constant.FMC_ERR_INFO_DICT
        elif err_bit == 25:
            mte_dict = Constant.FMD_ERR_INFO_DICT
        elif err_bit == 23:
            mte_dict = Constant.SOC_ERR_INFO_DICT
        elif err_bit == 21:
            mte_dict = Constant.AIPP_ERR_INFO_DICT
        else:
            mte_dict = {}

        if code in mte_dict:
            info = mte_dict.get(code)
        else:
            info = "NA"
        errinfo += f"\nmte_err_type bit[26:24]={code}  meaning:{info}"

        # mte_err_addr
        code = utils.get_01_from_hexstr(ret[0], 22, 8)
        info = "MTE Error Address [19:5]"
        # 补5位0，猜测值
        approximate = hex(int(code + "00000", 2))
        errinfo += f"\nmte_err_addr bit[22:8]={code}  meaning:{info}  approximate:{approximate}"
        return errinfo

    def _analyse_biu_errinfo(self: any) -> str:
        regexp = self.biu_key + r"=(\S+)"
        ret = re.findall(regexp, self.extra_info, re.M)
        if len(ret) == 0:
            return "No BIU_ERR_INFO found"

        errinfo = ret[0]
        # biu_err_addr
        code = utils.get_01_from_hexstr(ret[0], 24, 0)
        approximate = hex(int(code, 2))
        errinfo += f"\nbiu_err_addr bit[24:0]={code}  in hex:{approximate}"
        return errinfo

    def _analyse_ccu_errinfo(self: any) -> str:
        regexp = self.ccu_key + r"=(\S+)"
        ret = re.findall(regexp, self.extra_info, re.M)
        if len(ret) == 0:
            return "No CCU_ERR_INFO found"

        errinfo = ret[0]
        # ccu_err_addr
        code = utils.get_01_from_hexstr(ret[0], 22, 8)
        info = "CCU Error Address [17:3]"
        # 补3位0，猜测值
        approximate = hex(int(code + "000", 2))
        errinfo += f"\nccu_err_addr bit[22:8]={code}  meaning:{info}  approximate:{approximate}"
        return errinfo

    def _analyse_cube_errinfo(self: any) -> str:
        regexp = self.cube_key + r"=(\S+)"
        ret = re.findall(regexp, self.extra_info, re.M)
        if len(ret) == 0:
            return "No CUBE_ERR_INFO found"

        errinfo = ret[0]
        # cube_err_addr
        code = utils.get_01_from_hexstr(ret[0], 16, 8)
        info = "CUBE Error Address [17:9]"
        # 补9位0，猜测值
        approximate = hex(int(code + "000000000", 2))
        errinfo += f"\ncube_err_addr bit[16:8]={code}  meaning:{info}  approximate:{approximate}"
        return errinfo

    def _analyse_vec_errinfo(self: any) -> str:
        regexp = self.vec_key + r"=(\S+)"
        ret = re.findall(regexp, self.extra_info, re.M)
        if len(ret) == 0:
            return "No VEC_ERR_INFO found"

        errinfo = ret[0]
        # vec_err_addr
        code = utils.get_01_from_hexstr(ret[0], 28, 16)
        info = "VEC Error Address [17:5]"
        # 补5位0，猜测值
        approximate = hex(int(code + "00000", 2))
        errinfo += f"\nvec_err_addr bit[28:16]={code}  meaning:{info}  approximate:{approximate}"

        # vec_err_rcnt
        code = utils.get_01_from_hexstr(ret[0], 15, 8)
        info = "VEC Error repeat count [7:0]"
        repeats = str(int(code, 2))
        errinfo += f"\nvec_err_rcnt bit[15:8]={code}  meaning:{info}  repeats:{repeats}"
        return errinfo
