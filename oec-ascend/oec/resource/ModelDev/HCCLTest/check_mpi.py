#!/usr/bin/env python3
import subprocess
import sys
import os

def get_mpi_env():
    """获取运行mpirun所需的环境变量"""
    env = os.environ.copy()
    mpi_lib_path = "/usr/local/openmpi/lib"
    
    # 添加LD_LIBRARY_PATH如果目录存在
    if os.path.exists(mpi_lib_path):
        if "LD_LIBRARY_PATH" in env:
            env["LD_LIBRARY_PATH"] = f"{mpi_lib_path}:{env['LD_LIBRARY_PATH']}"
        else:
            env["LD_LIBRARY_PATH"] = mpi_lib_path
    return env

def check_mpi_installation():
    """
    检查MPI安装情况：
    - 返回 'correct' 如果是OpenMPI 4.1.5
    - 返回 'mpich' 如果是MPICH
    - 返回 'wrong_version' 如果是其他版本的OpenMPI
    - 返回 'other_mpi' 如果是其他MPI实现
    - 返回 'not_installed' 如果未安装
    """
    try:
        result = subprocess.run(
            ["mpirun", "--version"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=True,
            env=get_mpi_env()  # 使用修改后的环境变量
        )
        output = result.stdout
        
        # 添加调试输出，显示实际的命令输出
        print(f"mpirun --version 输出:\n{output}")
        
        if "MPICH" in output or "HYDRA" in output:
            return "mpich"
        
        # 直接匹配完整的版本号字符串，不提取版本号
        if "Open MPI" in output:
            if "mpirun (Open MPI) 4.1.5" in output:
                return "correct"
            else:
                return "wrong_version"
        
        return "other_mpi"
        
    except FileNotFoundError:
        return "not_installed"
    except subprocess.CalledProcessError as e:
        print(f"执行mpirun出错: {e.stderr}", file=sys.stderr)
        return "error"

def prepare_install_script(script_path):
    """准备安装脚本：修复格式和权限"""
    try:
        with open(script_path, 'r+', encoding='utf-8') as f:
            content = f.read()
            content = content.replace('\r\n', '\n').replace('\r', '\n')
            if not content.startswith("#!/"):
                content = "#!/bin/bash\n" + content
            f.seek(0)
            f.write(content)
            f.truncate()
        
        os.chmod(script_path, 0o755)
        return True
    except Exception as e:
        print(f"准备安装脚本失败: {str(e)}", file=sys.stderr)
        return False

def execute_install_script(script_path, data_dir, n_value):
    """执行安装脚本并实时显示输出"""
    try:
        process = subprocess.Popen(
            ["bash", script_path, data_dir, str(n_value)],
            stdout=None,
            stderr=None,
            bufsize=1,
            universal_newlines=True,
            env=get_mpi_env()  # 安装脚本也使用相同的环境变量
        )
        
        process.wait()
        return process.returncode == 0
        
    except Exception as e:
        print(f"执行安装脚本出错: {str(e)}", file=sys.stderr)
        return False

def main():
    if len(sys.argv) != 3:
        print("Usage: python check_mpi.py <data_dir> <n>")
        print("  <data_dir>: OpenMPI源码目录路径")
        print("  <n>: 安装并行数")
        sys.exit(3)
    
    data_dir = sys.argv[1]
    try:
        n_value = int(sys.argv[2])
    except ValueError:
        print("错误: 第二个参数必须是整数", file=sys.stderr)
        sys.exit(3)

    if not os.path.isdir(data_dir):
        print(f"错误: 目录不存在 '{data_dir}'", file=sys.stderr)
        sys.exit(3)

    status = check_mpi_installation()
    
    if status == "correct":
        print("✓ OpenMPI 4.1.5 已正确安装")
        sys.exit(0)
    
    if status in ("mpich", "wrong_version", "other_mpi", "error"):
        print(f"错误: 检测到不兼容的MPI安装 ({status})", file=sys.stderr)
        sys.exit(1)
    
    if status == "not_installed":
        print("未检测到OpenMPI，开始安装...")
        install_script = "./install_mpi.sh"
        
        if not os.path.exists(install_script):
            print(f"错误: 安装脚本不存在 {install_script}", file=sys.stderr)
            sys.exit(2)
        
        if not prepare_install_script(install_script):
            print("错误: 无法准备安装脚本", file=sys.stderr)
            sys.exit(2)
        
        print(f"正在执行安装脚本: {install_script} {data_dir} {n_value}")
        if not execute_install_script(install_script, data_dir, n_value):
            print("错误: 安装失败", file=sys.stderr)
            sys.exit(2)
        
        print("安装完成，正在验证...")
        post_status = check_mpi_installation()
        if post_status == "correct":
            print("✓ OpenMPI 4.1.5 安装成功")
            sys.exit(0)
        elif post_status == "mpich":
            print("错误: 安装结果居然是MPICH而不是OpenMPI", file=sys.stderr)
            sys.exit(2)
        else:
            print(f"错误: 安装后验证失败 ({post_status})", file=sys.stderr)
            sys.exit(2)
    
    print("未知错误", file=sys.stderr)
    sys.exit(3)

if __name__ == "__main__":
    main()