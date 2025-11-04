import sys
import subprocess
from packaging.version import parse as parse_version

def get_python_version():
    """获取当前Python版本字符串"""
    return f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"

def get_installed_packages():
    """获取所有已安装的包及其版本 (使用pip list)"""
    try:
        result = subprocess.run(
            [sys.executable, '-m', 'pip', 'list', '--format=freeze'],
            capture_output=True,
            text=True,
            check=True
        )
        packages = {}
        for line in result.stdout.splitlines():
            if '==' in line:
                name, version = line.split('==', 1)
                packages[name.lower()] = version.strip()
        return packages
    except Exception as e:
        print(f"错误: 无法获取已安装包列表 - {str(e)}")
        print("请确保pip已安装并能正常工作")
        sys.exit(1)

def check_python_version(min_version=None, max_version=None):
    """
    检查Python版本是否在指定范围内
    
    参数:
        min_version (str): 最小支持版本 (e.g., "3.8.0")
        max_version (str): 最大支持版本 (e.g., "3.10.0")
        
    返回:
        tuple: (是否满足, 问题描述)
    """
    current_ver = parse_version(get_python_version())
    problems = []
    
    if min_version:
        min_ver = parse_version(min_version)
        if current_ver < min_ver:
            problems.append(f"需要 ≥ {min_version}")
    
    if max_version:
        max_ver = parse_version(max_version)
        if current_ver > max_ver:
            problems.append(f"需要 ≤ {max_version}")
    
    return (len(problems) == 0, problems)

def check_package(pkg_info, installed_packages):
    """
    检查单个包是否满足要求
    
    参数:
        pkg_info (dict): 包配置信息
        installed_packages (dict): 已安装包的字典
        
    返回:
        tuple: (是否满足, 安装的版本, 问题描述)
    """
    pypi_name:str = pkg_info["pypi_name"].lower()
    installed_version = installed_packages.get(pypi_name)
    if not installed_version:
        installed_version = installed_packages.get(pypi_name.replace('_', '-'))
    # 包未安装
    if not installed_version:
        return (False, None, ["未安装"])
    
    # 没有版本要求
    if "min_version" not in pkg_info and "max_version" not in pkg_info:
        return (True, installed_version, [])
    
    # 检查版本要求
    problems = []
    try:
        installed_ver = parse_version(installed_version)
        
        if "min_version" in pkg_info:
            min_ver = parse_version(pkg_info["min_version"])
            if installed_ver < min_ver:
                problems.append(f"需要 ≥ {pkg_info['min_version']}")
        
        if "max_version" in pkg_info:
            max_ver = parse_version(pkg_info["max_version"])
            if installed_ver > max_ver:
                problems.append(f"需要 ≤ {pkg_info['max_version']}")
    except Exception as e:
        problems.append(f"版本解析错误: {str(e)}")
    
    return (len(problems) == 0, installed_version, problems)

def check_dependencies(requirements):
    """
    检查所有依赖项
    
    参数:
        requirements (dict): 依赖配置字典
        
    返回:
        tuple: (所有依赖是否满足, 包检查结果列表)
    """
    # 获取已安装包列表
    installed_packages = get_installed_packages()
    
    print("=" * 70)
    print("Python环境与包依赖检查")
    print("=" * 70)
    
    all_ok = True
    results = []
    
    # 1. 检查Python版本
    py_req = requirements.get("python", {})
    if py_req:
        min_py = py_req.get("min_version")
        max_py = py_req.get("max_version")
        py_ok, py_problems = check_python_version(min_py, max_py)

        version_range = []
        if min_py: version_range.append(f"≥ {min_py}")
        if max_py: version_range.append(f"≤ {max_py}")
        if min_py and max_py and min_py == max_py:
            range_str = min_py
        else:
            range_str = " 且 ".join(version_range) if version_range else "任意版本"
        
        status = "✓" if py_ok else "✗"
        problems = ", ".join(py_problems) if py_problems else "满足要求"
        print(f"Python版本: {get_python_version()} | 要求: {range_str}")
        print(f"  {status} 状态: {problems}")
        print("-" * 70)
        
        if not py_ok:
            all_ok = False
    
    # 2. 检查包依赖
    packages = requirements.get("packages", [])
    if not packages:
        print("未配置包依赖检查")
    else:
        print("\n包依赖检查:")
        print("-" * 70)
        
        for pkg in packages:
            # 获取包信息
            name = pkg["name"]
            pypi_name = pkg["name"]
            
            # 确定当前Python版本适用的规则
            current_py = f"{sys.version_info.major}.{sys.version_info.minor}"
            version_rules = pkg.get("version_rules", {})
            rule = version_rules.get(current_py, pkg.get("default", {}))
            
            # 检查包
            satisfied, version, problems = check_package(
                {"pypi_name": pypi_name, **rule},
                installed_packages
            )
            
            # 确定显示的要求范围
            
            range_parts = []
            if "min_version" in rule: 
                range_parts.append(f"≥ {rule['min_version']}")
            if "max_version" in rule: 
                range_parts.append(f"≤ {rule['max_version']}")
            if len(range_parts) == 2 and rule['min_version'] == rule['max_version']:
                range_str = rule['min_version']
            else:
                range_str = " 且 ".join(range_parts) if range_parts else "任意版本"
            
            # 确定状态
            if not satisfied:
                status = "✗"
                all_ok = False
            else:
                status = "✓"
            
            # 收集结果
            results.append({
                "display_name": name,
                "pypi_name": pypi_name,
                "status": status,
                "installed": version or "未安装",
                "required": range_str,
                "problems": problems,
                "rule": rule
            })
            
            # 打印结果
            print(f"{status} {name}")
            print(f"  已安装: {version or '未安装'}")
            print(f"  要求: {range_str}")
            if problems:
                print(f"  问题: {', '.join(problems)}")
            print("-" * 70)
    
    print("=" * 70)
    return all_ok, results

def generate_install_commands(results, py_req=None):
    """
    生成安装命令
    
    参数:
        results (list): 包检查结果列表
        py_req (dict): Python版本要求
        
    返回:
        str: 安装命令字符串
    """
    commands = []
    
    # Python版本要求
    if py_req:
        min_py = py_req.get("min_version")
        max_py = py_req.get("max_version")
        if min_py or max_py:
            commands.append("# 请确保使用正确的Python版本")
            if min_py and max_py:
                if max_py == min_py:
                    commands.append(f"# 推荐使用 Python {min_py}")
                else:
                    commands.append(f"# 推荐使用 Python {min_py} 到 {max_py} 之间的版本")
            elif min_py:
                commands.append(f"# 需要 Python {min_py} 或更高版本")
            elif max_py:
                commands.append(f"# 需要 Python {max_py} 或更低版本")
    
    # 包安装命令
    commands.append("\n# 包安装命令:")
    
    for res in results:
        pkg_name = res["pypi_name"]
        rule = res["rule"]
        
        if "min_version" in rule and "max_version" in rule:
            if rule['min_version']==rule['max_version']:
                commands.append(f"请先安装 '{pkg_name}=={rule['min_version']}'")
            else:
                commands.append(f"请先安装 '{pkg_name}>={rule['min_version']},<={rule['max_version']}'")
        elif "min_version" in rule:
            commands.append(f"请先安装 '{pkg_name}>={rule['min_version']}'")
        elif "max_version" in rule:
            commands.append(f"请先安装 '{pkg_name}<={rule['max_version']}'")
        else:
            commands.append(f"请先安装 {pkg_name}")
    
    return "\n".join(commands)

if __name__ == "__main__":
    # ====== 依赖配置 ======
    # 配置说明:
    #   - "python": 可选的Python版本要求
    #   - "packages": 包依赖列表
    #       每个包必须包含:
    #         - "name": PyPI上的包名
    #         - "version_rules": (可选) 针对不同Python版本的规则
    #         - "default": (可选) 默认规则
    #
    #   规则格式:
    #     {
    #       "min_version": "最低版本", 
    #       "max_version": "最高版本"
    #     }
    
    DEPENDENCY_CONFIG = {
        # Python版本要求
        "python": {
            "min_version": "3.7"
        },
        
        "packages": [
            {
                "name": "ais_bench_net_test"
            },
            
        ]
    }
    # ====================
    
    # 检查依赖
    all_ok, results = check_dependencies(DEPENDENCY_CONFIG)
    
    if all_ok:
        print("\n所有依赖满足! 可以运行主程序。")
        # 这里可以继续执行你的主程序
        # from main import main
        # main()
    else:
        print("\n错误: 环境不满足要求!")
        print("请根据以下提示解决问题:")
        
        # 生成安装建议
        py_req = DEPENDENCY_CONFIG.get("python", {})
        commands = generate_install_commands(results, py_req)
        print("\n" + commands)
        
        sys.exit(1)  # 非零退出码表示错误