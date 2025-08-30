import unittest
import numpy as np
import acl

def get_class_methods(class_name):
    method_list = [method.split("_") for method in dir(class_name) if method.startswith("test_")]
    method_list = sorted(method_list, key=lambda x: x[2])
    methods = ["_".join(method) for method in method_list]
    return methods

def switch_cases(case_class, opt):
    suite = unittest.TestSuite()
    methods = get_class_methods(case_class)

    if opt == "all":
        for method in methods:
            suite.addTest(case_class(method))
    return suite

def align_size(origin_size, alignment):
    if not alignment:
        return 0
    return ((origin_size + (alignment - 1)) // alignment) * alignment

def get_align_size(align_dict, pixel_fotmat, defaule_vale=0, case_value=0):
    for key in align_dict.keys():
        if pixel_fotmat in key:
            return align_dict.get(key)(defaule_vale, case_value)
    return defaule_vale

def get_device_type():
    device_type = acl.get_soc_name()[len('Ascend'):]
    if "P" in device_type:
        device_type = device_type[0:4]
    else:
        device_type = device_type[0:3]
    device_type = device_type == "910P" and "910" or device_type
    if device_type not in ["310", "310P", "910"]:
        raise Exception(f"device_type = {device_type} not in 310/310P/910, npu-smi not found!")
    return device_type

def params_check(test_case, param_dic, test_fun):
    parmas = param_dic['params']
    for _, param in enumerate(parmas):
        with test_case.assertRaises(TypeError):
            test_fun(*param)
    return 0