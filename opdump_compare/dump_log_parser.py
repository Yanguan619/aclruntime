import argparse
import os
import re
import time
import operator
import enum

from collections import namedtuple
from pathlib import Path

# ============================================== const template =====================================================
instr_info = namedtuple('instr_info', 'time, pipe_type, name')
pattern = re.compile(r'\[\w*]\s*\[(\w*)]\s*\(\w*\W*\w*\)\s*(\w*)\s*\W*\(\w*\W*\w*\)\s*(\w*).*')


# ========================================= Enum class for pipe type ================================================
class Pipe(enum.Enum):
    VEC = 1
    CUBE = 2
    MTE1 = 3
    MTE2 = 4
    MTE3 = 5
    FIXP = 6
    FC = 7
    SCALAR = 8
    ALL = 9


# ===================================== function for instr_log file parse ============================================
def parse_log(file_name):
    result = []
    with open(file_name, 'r') as f:
        for line in f.readlines():
            res = pattern.findall(line)
            if res:
                time = res[0][0]
                pipe_type = res[0][1]
                name = res [0][2]
                result.append(instr_info(time, pipe_type, name))

    return result


def parse_single_pipe(instr_list, pipe_type):
    pipe_list = []
    for instr in instr_list:
        if instr.pipe_type == pipe_type.name:
            pipe_list.append(instr)
    sorted_pipe = sorted(pipe_list, key=operator.attrgetter('time'))

    return sorted_pipe


def parse_pipes(instr_list):
    vec_pipe = parse_single_pipe(instr_list, Pipe.VEC)
    cube_pipe = parse_single_pipe(instr_list, Pipe.CUBE)
    mte1_pipe = parse_single_pipe(instr_list, Pipe.MTE1)
    mte2_pipe = parse_single_pipe(instr_list, Pipe.MTE2)
    mte3_pipe = parse_single_pipe(instr_list, Pipe.MTE3)
    fix_pipe = parse_single_pipe(instr_list, Pipe.FIXP)
    fc_pipe = parse_single_pipe(instr_list, Pipe.FC)
    scalar_pipe = parse_single_pipe(instr_list, Pipe.SCALAR)
    all_pipe = parse_single_pipe(instr_list, Pipe.ALL)

    return [vec_pipe, cube_pipe, mte1_pipe, mte2_pipe, mte3_pipe, fix_pipe, fc_pipe, scalar_pipe, all_pipe]


# ===================================== function for IO ============================================
def write_to_file(file, instr_list):
    lines = []
    for instr in instr_list:
        lines.append(instr.time + '\t' + instr.pipe_type + '\t' + instr.name + '\n')
    with open(file, 'w') as f:
        f.writelines(lines)


def make_new_folder(folder):
    parent_folder = Path(folder)
    timestamp = time.strftime('%Y%m%d%H%M%S', time.localtime())
    new_folder = parent_folder.joinpath(timestamp)
    os.makedirs(new_folder)

    return new_folder


def write_pipe_to_file(folder_path, instr_list, pipe_type):
    output_path = Path.joinpath(folder_path, pipe_type.name + '.txt')
    write_to_file(output_path, instr_list)


# ===================================== arg parser ============================================
def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', dest='input', help='Dump file to parse.Which named like \'core0.vectorcore0.instr_log.dump\'.')
    parser.add_argument('-o', '--output', dest='output', help='Path to save result.')
    args = parser.parse_args()

    return args


# ===================================== main entry ============================================
if __name__ == '__main__':
    args = parse_args()
    instr_list = parse_log(args.input)
    new_folder = make_new_folder(args.output)
    full_output_path = Path.joinpath(new_folder, 'FULL_ORDER.txt')
    write_to_file(full_output_path, instr_list)
    for pipe_type in Pipe:
        pipe_list = parse_single_pipe(instr_list, pipe_type)
        write_pipe_to_file(new_folder, pipe_list, pipe_type)
