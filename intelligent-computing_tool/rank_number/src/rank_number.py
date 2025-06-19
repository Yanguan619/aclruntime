import re

def extract_rank_info_and_num(log_file_path):
       with open(log_file_path, 'r') as file:
                log_content=file.read()

       # 使用正则表达式匹配每行的rank信息
       rank_pattern=r'\[(\d{16})\]'
       rank_matches=re.findall(rank_pattern, log_content)

       # 使用正则表达式匹配每行的ranknum信息
       num_pattern=r'rankNum\[(\d+)\]'
       num_match=re.search(num_pattern, log_content)

       if not num_match
            raise ValueError('rankNum not found in the log file.')

       rank_num=int(num_match.group(1))

       #解析匹配结果，去除多余的空格和换行符
       rank_infos=[int(match) for match in rank_matches]
       rank_infos.sort() #对提取到的rank信息进行排序

       return rank_infos, rank_num

def find_missing_ranks(rank_infos, rank_num):
       if not rank_infos:
           return []

       #生成完整的rank序列
       full_rank_sequence=list(range(0, rank_num))

       #找出缺失的rank
       missing_ranks= set(full_rank_sequence).difference(set(rank_infos))

       return sorted(missing_ranks)

#日志路径
log_file_path='xxx.log'
rank_infos,rank_num= extract_rank_info_and_num(log_file_path)

missing_ranks = find_missing_ranks(rank_infos, rank_num)

print("缺失的rank信息：")
for missing_rank in missing_ranks:
      print(f"{missing_rank:016d}")

