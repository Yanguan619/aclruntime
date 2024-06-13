import numpy as np
import os
import argparse
import pandas as pd

node_map = {"Gather": "GatherV2",
            "Matmul": "MatmulV2",
            "Slice": "StridedSliceV3"}
input_node_list = ["Data", "ConstPlaceHolder", "Constant", "NetOutput", "Const", "Variable", "PartitionedCall"]
def parser_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--origin_graph", type=str, action='store', required=False, default='', help='origin graph file name')
    parser.add_argument("--compiled_graph", type=str, action='store', required=False, default='', help='compiled graph file name')
    parser.add_argument("--single_op", type=str, action='store', required=False, default='', help='profiling data in single op mode')
    parser.add_argument("--graph_execute", type=str, action='store', required=False, default='', help='profiling data in graph mode')
    args = parser.parse_args()
    return args

def readfile(filename):
    with open(filename, "r") as f:
        data = f.readlines()
        return data

def handle_graph(filename):
    file_data = readfile(filename)
    data_split_list = []
    for i in range(1, len(file_data) - 1):
        if file_data[i].startswith("    name:") == True and file_data[i - 1].startswith("  op {") == True:
            type_line = file_data[i + 1].split('"')[1]
            if type_line in input_node_list:
                continue
            data_line = [file_data[i].split('"')[1], file_data[i + 1].split('"')[1]]
            data_split_list.append(data_line)
    write_file_list(data_split_list, filename)
    return data_split_list

def handle_sequence(filename):
    excel_data = pd.read_csv(filename, usecols=['Name', 'Type'])
    data_split_list=[]
    for i in range(len(excel_data['Name'].values)):
        name_and_type = [excel_data['Name'].values[i], excel_data['Type'].values[i]]
        data_split_list.append(name_and_type)
    write_file_list(data_split_list, filename)
    return data_split_list

def compare_graph_topo(origin_node_name_list, last_node_name_list, calc_mode):
    origin_node_name_list.reverse()
    last_node_name_list.reverse()
    dfs_status_list = [[0] * (len(origin_node_name_list) + 1) for _ in range(len(last_node_name_list) + 1)]
    init_status = 0
    for i in range(len(origin_node_name_list) + 1):
        dfs_status_list[0][i] = init_status
        init_status = init_status - 1
    init_status = 0
    for i in range(len(last_node_name_list) + 1):
        dfs_status_list[i][0] = init_status
        init_status = init_status - 1
    for i in range(1, len(last_node_name_list) + 1):
        for j in range(1, len(origin_node_name_list) + 1):
            dfs_status_list[i][j] = max(dfs_status_list[i][j - 1] - 1, dfs_status_list[i - 1][j] - 1)
            match_score = 0
            if calc_mode == 0:
                if origin_node_name_list[j - 1][1] == last_node_name_list[i - 1][1]:
                    match_score = dfs_status_list[i - 1][j - 1] + 1
                else:
                    match_score = dfs_status_list[i - 1][j - 1] - 1
            elif calc_mode == 1:
                if origin_node_name_list[j - 1][1] in node_map and last_node_name_list[i - 1][1] == node_map[origin_node_name_list[j - 1][1]] or origin_node_name_list[j - 1][1] == last_node_name_list[i - 1][1]:
                    match_score = dfs_status_list[i - 1][j - 1] + 1
                else:
                    match_score = dfs_status_list[i - 1][j - 1] - 1
            dfs_status_list[i][j] = max(dfs_status_list[i][j], match_score)
    i = len(last_node_name_list)
    j = len(origin_node_name_list)
    after_origin_node_name_list = []
    after_last_node_name_list = []
    while i > 0 or j > 0:
        max_value = 0
        if i <= 0:
            max_value = dfs_status_list[i][j - 1]
        elif j <= 0:
            max_value = dfs_status_list[i - 1][j]
        else:
            max_value = max(dfs_status_list[i - 1][j - 1], dfs_status_list[i][j - 1], dfs_status_list[i - 1][j])
        
        if i > 0 and j > 0 and max_value == dfs_status_list[i - 1][j - 1]:
            after_origin_node_name_list.append(origin_node_name_list[j - 1])
            after_last_node_name_list.append(last_node_name_list[i - 1])
            j = j - 1
            i = i - 1
        elif i > 0 and max_value == dfs_status_list[i - 1][j]:
            after_origin_node_name_list.append(["-----", "-----"])
            after_last_node_name_list.append(last_node_name_list[i - 1])
            i = i - 1
        elif j > 0 and max_value == dfs_status_list[i][j - 1]:
            after_origin_node_name_list.append(origin_node_name_list[j - 1])
            after_last_node_name_list.append(["-----", "-----"])
            j = j - 1
    return after_origin_node_name_list, after_last_node_name_list

def compare_output_result(after_origin_node_name_list, after_last_node_name_list, calc_mode, origin_len):
    same_num = 0
    sim_num = 0
    same_result_list = []
    sim_result_list = []
    for i in range(len(after_origin_node_name_list)):
        if calc_mode == 0:
            if after_origin_node_name_list[i][1] == after_last_node_name_list[i][1]:
                same_num = same_num + 1
            else:
                diff_info = str(i + 1) + ": " + after_origin_node_name_list[i][0] + " " + after_origin_node_name_list[i][1] + " vs " + after_last_node_name_list[i][0] + " " + after_last_node_name_list[i][1] + "\n"
                same_result_list.append(diff_info)
        elif calc_mode == 1:
            if after_origin_node_name_list[i][1] in node_map and after_last_node_name_list[i][1] == node_map[after_origin_node_name_list[i][1]] or after_origin_node_name_list[i][1] == after_last_node_name_list[i][1]:
                sim_num = sim_num + 1
            else:
                diff_info = str(i + 1) + ": " + after_origin_node_name_list[i][0] + " " + after_origin_node_name_list[i][1] + " vs " + after_last_node_name_list[i][0] + " " + after_last_node_name_list[i][1] + "\n"
                sim_result_list.append(diff_info)
    if calc_mode == 0:
        same_num_str = "topo same node num: " + str(same_num) + "\n"
        per = "same per: " + str(same_num / origin_len) + "\n"
        same_result_list.append(same_num_str)
        same_result_list.append(per)
        print(same_num_str)
        print(per)
        write_file_data(same_result_list, "compare_result_same")
    if calc_mode == 1:
        sim_num_str = "topo sim node num: " + str(sim_num) + "\n"
        sim_per = "sim per: " + str(sim_num / origin_len) + "\n"
        sim_result_list.append(sim_num_str)
        sim_result_list.append(sim_per)
        print(sim_num_str)
        print(sim_per)
        write_file_data(sim_result_list, "compare_result_sim")
    return

def get_compare_sequence(args):
    origin_sequence = []
    last_sequence = []
    if args.origin_graph == "" and args.single_op == "" or args.origin_graph != "" and args.single_op != "":
        print("--orgin_graph and --single_op only one of them can have value")
        return origin_sequence, last_sequence
    if args.compiled_graph == "" and args.graph_execute == "" or args.compiled_graph != "" and args.graph_execute != "":
        print("--compiled_graph and --graph_execute only one of them can have value")
        return origin_sequence, last_sequence
    if args.origin_graph != "":
        origin_sequence = handle_graph(args.origin_graph)
    if args.compiled_graph != "":
        last_sequence = handle_graph(args.compiled_graph)
    if args.single_op != "":
        origin_sequence = handle_sequence(args.single_op)
    if args.graph_execute != "":
        last_sequence = handle_sequence(args.graph_execute)
    return origin_sequence, last_sequence
def create_path(filepath):
    folder = os.path.exists(filepath)
    if not folder:
        os.makedirs(filepath)
    return

def write_file_data(sequence, filename):
    write_filename = "./compare_result/" + filename.split('.')[0] + ".txt"
    with open(write_filename, "w") as f:
        for s in sequence:
            f.write(s)
    return

def write_file_list(sequence, filename):
    write_filename = "./compare_result/" + filename.split('.')[0] + "_type.txt"
    with open(write_filename, "w") as f:
        for s in sequence:
            node_type = s[1] + '\n'
            f.write(node_type)
    write_filename = "./compare_result/" + filename.split('.')[0] + "_name.txt"
    with open(write_filename, "w") as f:
        for s in sequence:
            node_name = s[0] + '\n'
            f.write(node_name)
    return

def main():
    args = parser_args()
    create_path("./compare_result")
    origin_sequence, last_sequence = get_compare_sequence(args)
    after_origin_node_name_list, after_last_node_name_list = compare_graph_topo(origin_sequence, last_sequence, 0)
    compare_output_result(after_origin_node_name_list, after_last_node_name_list, 0, len(last_sequence))
    sim_after_origin_node_name_list, sim_after_last_node_name_list = compare_graph_topo(origin_sequence, last_sequence, 1)
    compare_output_result(sim_after_origin_node_name_list, sim_after_last_node_name_list, 1, len(last_sequence))
    print("compare sequence success")
    print("result is in ./compare_result")

main()