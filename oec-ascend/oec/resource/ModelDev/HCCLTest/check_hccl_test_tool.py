import os
import sys

def main():
    # 获取环境变量
    ascend_home = os.environ.get('ASCEND_HOME_PATH')
    if not ascend_home:
        print("环境变量 ASCEND_HOME_PATH 未设置")
        sys.exit(1)
    
    # 定义需要检查的路径
    paths_to_check = [
        ascend_home,  # 主目录
        os.path.join(ascend_home, "tools/hccl_test/bin"),  # 工具目录
        os.path.join(ascend_home, "tools/hccl_test/bin/all_gather_test"),  # 测试文件
        os.path.join(ascend_home, "tools/hccl_test/bin/all_reduce_test"),  # 测试文件
        os.path.join(ascend_home, "tools/hccl_test/bin/broadcast_test")   # 测试文件
    ]
    
    # 检查每个路径
    results = {}
    for path in paths_to_check:
        if os.path.exists(path):
            # 区分目录和文件
            if os.path.isdir(path):
                results[path] = "目录存在"
            else:
                results[path] = "文件存在"
        else:
            results[path] = "不存在"
    
    # 打印结果
    for path, status in results.items():
        print(f"{path}: {status}")
    
    # 如果有缺失则返回错误码
    if any("不存在" in s for s in results.values()):
        sys.exit(1)

if __name__ == "__main__":
    main()