import os
import re

def round_time_to_2_decimal(match):
    hours, minutes, seconds, milliseconds = match.groups()
    total_seconds = int(seconds) + int(milliseconds) / 1000  # 转换为秒
    rounded_seconds = round(total_seconds, 2)  # 四舍五入保留两位小数
    return f"{int(hours)}:{minutes}:{rounded_seconds:05.2f}"  # 确保两位小数

def process_ass_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    pattern = re.compile(r"(\d+):(\d+):(\d+)\.(\d{3})")
    new_content = pattern.sub(round_time_to_2_decimal, content)
    
    if new_content != content:  # 只有内容变化才更新文件
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"修改了文件: {file_path}")

def batch_process_ass_files(folder_path):
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file.endswith(".ass"):
                process_ass_file(os.path.join(root, file))

if __name__ == "__main__":
    folder_path = input("请输入包含 .ass 文件的文件夹路径: ").strip()
    if os.path.isdir(folder_path):
        batch_process_ass_files(folder_path)
        print("处理完成。")
    else:
        print("无效的文件夹路径。")
