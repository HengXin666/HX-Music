import re
import os
import pykakasi
from typing import List


def is_kanji(char: str) -> bool:
    """判断字符是否为汉字"""
    cp = ord(char)
    return (0x4E00 <= cp <= 0x9FFF or    # CJK统一汉字
            0x3400 <= cp <= 0x4DBF or   # 扩展A
            0x20000 <= cp <= 0x2A6DF or # 扩展B
            0x2A700 <= cp <= 0x2B73F or # 扩展C
            0x2B740 <= cp <= 0x2B81F or # 扩展D
            0x2B820 <= cp <= 0x2CEAF or # 扩展E
            0x2CEB0 <= cp <= 0x2EBEF)   # 扩展F

def convert_time(match: re.Match) -> str:
    """处理时间格式转换"""
    time_str = match.group()
    h, m, s_ms = time_str.split(':')
    s, ms = s_ms.split('.')
    
    total_seconds = int(h)*3600 + int(m)*60 + int(s) + int(ms)/1000
    rounded = round(total_seconds, 2)
    
    new_h = int(rounded // 3600)
    remaining = rounded % 3600
    new_m = int(remaining // 60)
    new_s = remaining % 60
    
    return f"{new_h}:{new_m:02}:{new_s:05.2f}"

def add_ruby(text: str, kks) -> str:
    """为汉字添加注音"""
    if '|' in text:
        return text
    
    converted = kks.convert(text)
    result = []
    for item in converted:
        orig = item['orig']
        hira = item['hira']
        
        # 如果包含汉字且假名与汉字不同
        if any(is_kanji(c) for c in orig) and orig != hira:
            result.append(f"{orig}|&lt;{hira}")
        else:
            result.append(orig)
    return ''.join(result)

def process_line(line: str, kks) -> str:
    """处理单行内容"""
    # 处理时间
    new_line = re.sub(r'\d+:\d{2}:\d{2}\.\d{3}', convert_time, line)
    
    # 分割标签和文本
    parts = re.split(r'({.*?})', new_line)
    for i in range(len(parts)):
        # 只处理非标签部分且包含\kf的段落
        if not parts[i].startswith('{') and '\\kf' in new_line:
            parts[i] = add_ruby(parts[i], kks)
    return ''.join(parts)

def process_file(file_path: str) -> bool:
    """处理单个文件"""
    kks = pykakasi.kakasi()
    kks.setMode('J', 'H')  # 汉字转平假名
    kks.setMode('K', 'H')  # 片假名转平假名
    kks.setMode('H', 'H')  # 平假名保留
    
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    modified = False
    new_lines = []
    
    for line in lines:
        original = line
        processed = process_line(line.strip(), kks)
        new_lines.append(processed + '\n')
        if processed != original.strip():
            modified = True
    
    if modified:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
    return modified

def main(folder_path: str):
    """主函数"""
    processed = []
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file.lower().endswith('.ass'):
                path = os.path.join(root, file)
                if process_file(path):
                    processed.append(path)
    
    if processed:
        print("已修改以下文件:")
        for p in processed:
            print(p)
    else:
        print("没有需要修改的文件")

if __name__ == '__main__':
    kks = pykakasi.kakasi()
    kks.setMode('J', 'H')  # 汉字转平假名
    # kks.setMode('K', 'H')  # 片假名转平假名
    # kks.setMode('H', 'H')  # 平假名保留
    # testStr = "{\kf40}微|び{\kf25}笑|わらい{\kf35}ん{\kf40}で"
    testStr = "微笑んで"
    res = kks.convert(testStr)[0]["hira"]
    print(res)

        # import sys
    # if len(sys.argv) != 2:
    #     print("请将文件夹拖放到本脚本上")
    #     sys.exit(1)
    # 创建一个pykakasi实例
    kks = pykakasi.kakasi()

    # 要处理的日语文本
    text = "微笑んで"

    # 转换为假名
    result = kks.convert(text)
    print(result)
    # 准备输出的格式
    annotated_text = ''

    # 遍历转换后的结果
    for item in result:
        original = item['orig']  # 原始字符
        romaji = item['hira']  # 罗马字注音
        # 例如：微(hoho)
        annotated_text += f"{original}({romaji})"

    # 输出结果
    print(annotated_text)
    # main(sys.argv[1])
    pass