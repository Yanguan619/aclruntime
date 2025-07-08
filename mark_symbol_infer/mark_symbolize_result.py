import regex
import sys
from pathlib import Path

def process_graph_file(input_file, output_file):
    # 读取输入文件内容
    with open(input_file, 'r') as f:
        content = f.read()
    
    # 定义递归匹配node块的正则表达式
    node_pattern = r'node\s*(?P<node_body>\{(?:[^{}]|(?P>node_body))*\})'
    
    # 处理每个node块
    def process_node(match):
        full_match = match.group(0)
        node_body = match.group('node_body')
        
        # 检查属性
        has_output_attr = 'output_desc_attr_groups' in node_body
        has_input_attr = 'input_desc_attr_groups' in node_body
        
        # 确定要添加的前缀和属性值
        if has_output_attr:
            prefix = "symbol_"
            attr_value = 0
        elif has_input_attr:
            prefix = "symbol_break_"
            attr_value = 1
        else:
            prefix = "nosymbol_"
            attr_value = 2
        
        # 修改节点名称
        name_match = regex.search(r'name:\s*"([^"]+)"', full_match)
        if name_match:
            original_name = name_match.group(1)
            new_name = prefix + original_name
            full_match = full_match.replace(
                f'name: "{original_name}"', 
                f'name: "{new_name}"'
            )
            # 更新node_body以包含新名称
            node_body = node_body.replace(
                f'name: "{original_name}"', 
                f'name: "{new_name}"'
            )
        
        # 添加新属性
        new_attribute = f'''  attribute {{
      name: "symbol_infer_shape_result"
      i: {attr_value}
      type: INT
    }}
'''
        
        # 在node_body中找到最后一个属性的位置
        last_attr_match = regex.search(
            r'(attribute\s*\{[^{}]*\}(?:\s*[^{}]?)*)\s*\}$', 
            node_body, 
            flags=regex.DOTALL
        )
        
        if last_attr_match:
            # 在最后一个属性后面插入新属性
            last_attr_end = last_attr_match.end(1)
            new_body = (
                node_body[:last_attr_end] + 
                new_attribute + 
                node_body[last_attr_end:]
            )
        else:
            # 如果没有属性，直接添加到内容开头
            new_body = new_attribute + '\n' + node_body
        
        # 重新构建完整的node块
        return full_match.replace(node_body, new_body)
    
    # 处理所有node块
    processed_content = regex.sub(
        node_pattern, 
        process_node, 
        content, 
        flags=regex.DOTALL
    )
    
    # 写入输出文件
    with open(output_file, 'w') as f:
        f.write(processed_content)
    
    print(f"处理完成！结果已保存到 {output_file}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("用法：python mark_symbolize_result.py <文件路径>")
        sys.exit()

    file_path_str = sys.argv[1]
    file_path = Path(file_path_str)

    if not file_path.exists:
        print(f"文件不存在: {file_path_str}")
        sys.exit()

    file_name = file_path.name
    if not file_name.startswith("ge_onnx"):
        print("仅支持onnx格式的dump图,请检查文件")
        sys.exit()
    if not file_name.endswith("pbtxt"):
        print("仅支持pbtxt文件，请检查文件")
        sys.exit()
    
    output_file_path = str(Path.cwd()) + "/" + file_path.stem + "_mark_symbol_infer.pbtxt"
    
    # 处理文件
    process_graph_file(file_path_str, output_file_path)
    