# Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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
import argparse
import re
from ais_bench.net_test.common.consts import IntLimit, LengthLimit, StringPattern
from ais_bench.net_test.common.utils import get_actual_device_count, compare_bytes_string
from ais_bench.net_test.security.file_checker import check_linux_readable_file
from ais_bench.net_test.security.standard_consts import PermForbid


def arg_check_hostfile_legalty(value):
    try:
        check_linux_readable_file(value, perm_forbid=PermForbid.SECRET_FILE)
    except Exception as err:
        raise argparse.ArgumentTypeError(f"hostfile does not pass security check! detail: {err}")
    return value


def arg_check_whl_legalty(value):
    try:
        check_linux_readable_file(value)
    except Exception as err:
        raise argparse.ArgumentTypeError(f"whl package path does not pass security check! detail: {err}")
    return value


def arg_check_ssh_key_path_legalty(value):
    try:
        check_linux_readable_file(value, perm_forbid=PermForbid.SECRET_FILE)
    except Exception as err:
        raise argparse.ArgumentTypeError(f"ssh key does not pass security check! detail: {err}")
    return value


def arg_check_ipv4_string(value):
    if len(value) > LengthLimit.MAX_IPV4_LENGTH:
        raise argparse.ArgumentTypeError(
            f"The length of ipv4_string is over MAX_IPV4_LENGTH {LengthLimit.MAX_IPV4_LENGTH}!"
        )
    if not re.match(StringPattern.LEGAL_IPV4_PATTERN, value):
        raise argparse.ArgumentTypeError(f"The format of ipv4_string:{value} is illegal!")
    return value


def arg_check_port_range(value):
    if (len(value) > LengthLimit.MAX_PORT_STR_LENGTH):
        raise argparse.ArgumentTypeError(f"The length of port str is over {LengthLimit.MAX_PORT_STR_LENGTH}!")

    arg_check_nonnegative_integer(value)

    ivalue = int(value)
    if ivalue > IntLimit.PORT_MAX:
        raise argparse.ArgumentTypeError(
            f"The port value:{value} is illegal, legal range is [0, {IntLimit.PORT_MAX}]"
        )
    return ivalue


def arg_check_positive_integer(value):
    if value is None:
        return
    if len(value) > LengthLimit.MAX_UINT64_STR_LENGTH:
        raise argparse.ArgumentTypeError(f"{value} is an invalid positive int value")
    if not value.isdigit():
        raise argparse.ArgumentTypeError(f"{value} is an invalid positive int value")
    ivalue = int(value)
    if ivalue == 0:
        raise argparse.ArgumentTypeError("%s is an invalid positive int value" % value)


def arg_check_nonnegative_integer(value):
    if value is None:
        return
    if len(value) > LengthLimit.MAX_UINT64_STR_LENGTH:
        raise argparse.ArgumentTypeError(f"{value} is an invalid positive int value")
    if not value.isdigit():
        raise argparse.ArgumentTypeError(f"{value} is an invalid positive int value")
    ivalue = int(value)


def arg_check_bytes_format(value):
    if not (LengthLimit.MIN_BYTES_STR_LENGTH <= len(value) <= LengthLimit.MAX_BYTES_STR_LENGTH):
        raise argparse.ArgumentTypeError("Bytes str is not in legal length range " +
            f"[{LengthLimit.MIN_BYTES_STR_LENGTH}, {LengthLimit.MAX_BYTES_STR_LENGTH}]")
    legal_suffix = ["K", "M", "G"]
    if value[-1] not in legal_suffix:
        raise argparse.ArgumentTypeError(f"Bytes str should be end up with {legal_suffix}")
    value_int = value[:-1]
    arg_check_positive_integer(value_int)
    return value


def arg_check_device_count_range(value):
    arg_check_positive_integer(value)
    ivalue = int(value)
    actual_device_count = get_actual_device_count()
    if ivalue > actual_device_count:
        raise argparse.ArgumentTypeError(f"device_count should not be over {actual_device_count}")
    return ivalue


def arg_check_device_id_range(value):
    arg_check_nonnegative_integer(value)
    ivalue = int(value)
    actual_device_count = get_actual_device_count()
    if ivalue > actual_device_count:
        raise argparse.ArgumentTypeError(f"device_id should not be over {actual_device_count - 1}")
    return ivalue


def combination_args_check(args):
    check_node_id_smaller_than_rank_size(args)
    check_root_smaller_than_npus(args)
    check_minbytes_and_maxbytes(args)


def check_node_id_smaller_than_rank_size(args):
    if args.rank_size <= args.node_id:
        raise ValueError(f"node_id: {args.node_id} >= rank_size: {args.rank_size} !")


def check_root_smaller_than_npus(args):
    if args.npus <= args.root:
        raise ValueError(f"root id: {args.root} >= npus: {args.npus} !")


def check_minbytes_and_maxbytes(args):
    if not compare_bytes_string(args.minbytes, args.maxbytes):
        raise ValueError(f"minbytes: {args.minbytes} > maxbytes: {args.maxbytes} !")