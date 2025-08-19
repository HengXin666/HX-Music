from src.mark.AssMark import AssMark

import re
from typing import Callable

def processAssFile(inFile: str, outFile: str, callback: Callable[[str], str]):
    """
    inFile: 输入的 ass 文件路径
    outFile: 输出的 ass 文件路径
    callback: (raw: str) -> str, 回调函数, 接收原始 karaoke 序列字符串, 返回替换字符串
    """
    # 匹配连续的 karaoke 序列: {\\k...}{\\k...}...
    seqPattern = re.compile(r"(?:\{\\k[f|o]?\d+\}[^{]*)+")

    with open(inFile, "r", encoding="utf-8") as f:
        lines = f.readlines()

    outLines = []
    for line in lines:
        if line.startswith("Dialogue:"):
            # 第 9 个逗号后的部分就是 Text
            textPart = line.split(",", 9)[-1].split('\n')[0]

            # 找出所有序列并调用回调替换
            newText = textPart
            for m in seqPattern.finditer(textPart):
                raw = m.group(0)  # 原始字符串
                replaced = callback(raw)
                newText = newText.replace(raw, replaced, 1)

            head = line[:len(line) - len(textPart) - 1]
            newLine = head + newText
            outLines.append(f"{newLine}\n")
        else:
            outLines.append(line)

    with open(outFile, "w", encoding="utf-8") as f:
        f.writelines(outLines)


from src.mark.jpMark import JpMark

# ========== 使用示例 ==========
if __name__ == "__main__":
    # s = JpMark().convert('日々')
    # print(s)
    # exit(0)
    # 示例: 简单回调, 把所有 \kf 改为 \k
    def demoCallback(raw: str) -> str:
        return AssMark.mark(raw)

    processAssFile("test.ass", "output.ass", demoCallback)