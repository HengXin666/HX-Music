import os
import re
import pykakasi
from glob import glob

def process_time(match):
    time_part, ms_part = match.groups()
    h, m, s = map(int, time_part.split(':'))
    total_ms = h * 3600000 + m * 60000 + s * 1000 + int(ms_part)
    rounded_total = round(total_ms / 10) * 10  # 四舍五入到两位小数
    
    h = rounded_total // 3600000
    remainder = rounded_total % 3600000
    m = remainder // 60000
    remainder %= 60000
    s = remainder // 1000
    cs = (remainder % 1000) // 10
    
    return f"{h}:{m:02d}:{s:02d}.{cs:02d}"

def process_ruby(text, kks):
    def replacer(match):
        tags = re.findall(r'\\{\\kf\d+\\}', match.group())
        chars = re.findall(r'}([^\\{]+)', match.group())
        full_text = ''.join(chars)
        
        converted = kks.convert(full_text)
        ruby = []
        ptr = 0
        
        for item in converted:
            orig = item['orig']
            hira = item['hira']
            
            # 分割假名以匹配原始字符
            while len(orig) > 0 and ptr < len(chars):
                char = chars[ptr]
                if orig.startswith(char):
                    ruby.append(f"{char}({hira[:len(char)]})")
                    hira = hira[len(char):]
                    orig = orig[len(char):]
                    ptr += 1
                else:
                    break
        
        # 重新组合带标签的文本
        result = []
        for i in range(len(chars)):
            if i < len(ruby):
                result.append(f"{tags[i]}{ruby[i]}")
            else:
                result.append(f"{tags[i]}{chars[i]}")
        
        return ''.join(result)

    return re.sub(r'(\\{\\kf\d+\\}[^\\{]+)+', replacer, text)

def process_file(filepath, content):
    if (filepath != None):
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()

    # 处理时间
    time_pattern = re.compile(r'(\d:\d{2}:\d{2})\.(\d{3})')
    new_content, time_count = time_pattern.subn(process_time, content)
    modified = time_count > 0

    # 处理注音
    kks = pykakasi.kakasi()
    new_content = process_ruby(new_content, kks)
    ruby_modified = new_content != content

    if modified or ruby_modified:
        if (filepath != None):
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(new_content)
            print(f"已更新文件: {os.path.basename(filepath)}")
        print(new_content)
    else:
        print(f"无需修改: {os.path.basename(filepath)}")

def main():
    folder = input("请输入文件夹路径: ")
    for filepath in glob(os.path.join(folder, '*.ass')):
        process_file(filepath)

if __name__ == "__main__":
    # main()
    process_file(None, "Dialogue: 0,00:00:16.223,00:00:21.379,orig,,0,0,0,,{\kf40}果{\kf60}て{\kf60}は{\kf35}懐{\kf20}か{\kf20}し{\kf25}い{\kf60}季{\kf135}節")
