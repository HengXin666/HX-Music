import re
from typing import List

from src.mark.jpMark import JpMark

mark = JpMark()

# ラストチャンスに飢えたつま先が、踊り出すまま駆けたこの夜空
# print(mark.convert("ラストチャンスに飢えたつま先が、踊り出すまま駆けたこの夜空"))

# {\kf16}ラ{\kf13}ス{\kf15}ト{\kf11}チ{\kf11}ャ{\kf11}ン{\kf20}ス{\kf20}に{\kf18}飢{\kf20}え{\kf20}た{\kf22}つ{\kf20}ま{\kf39}先{\kf21}が

# 为了词法分析, 应该先去掉 {\kf%d+}, 复原为一句话
# 然后进行注音, 之后, 双指针进行

# 复原需要带 kf, 定义一个词组为 {\kf%d+}%s
# 这样可以通过内置注音平局切分了

class KfToken:
    def __init__(self, kf: int = 0, text: str = "") -> None:
        self.kf: int = kf
        self.str: str = text

    def __repr__(self) -> str:
        return f"KfToken(kf={self.kf}, str='{self.str}')"

def parseKfLine(assLine: str) -> List[KfToken]:
    """
    将 ASS 注音行解析成 KfToken 列表
    每个 KfToken 包含 kf 时间和对应字符 (可能多个字符)
    """
    pattern = r'\{\\kf(\d+)\}([^\{]+)'
    tokens = []
    for match in re.findall(pattern, assLine):
        kf_time = int(match[0])
        text = match[1]
        tokens.append(KfToken(kf_time, text))
    return tokens

def toLinkStr(assLine: str) -> str:
    # 匹配 {\kf数字} 后面的所有非 k 帧标记内容
    pattern = r'\{\\kf\d+\}([^\{]+)'
    return "".join(re.findall(pattern, assLine))

assLine = r"{\kf16}ラ{\kf13}ス{\kf15}ト{\kf11}チ{\kf11}ャ{\kf11}ン{\kf20}ス{\kf20}に{\kf18}飢{\kf20}え{\kf20}た{\kf22}つ{\kf20}ま{\kf39}先{\kf21}が"

# 期望输入是逐字歌词
lineStr = toLinkStr(assLine)        # 飢えたつま先が
markList = mark.convert(lineStr)    # 飢えた | つま | 先 | が
kfTokenList = parseKfLine(assLine)  # 飢 | え | た | つ | ま | 先 | が

res = []

i = 0
j = 0
k = 0

nowMarkPos = 0

markLen = len(markList)
n = len(lineStr)

while i < markLen:
    # 一般情况下 len: mark >= kfToken.str
    mark = markList[i]
    kfToken = kfTokenList[j]

    pos = len(kfToken.str)
    nowMarkPos += pos
    motoMarkLen = len(mark[0])
    if (nowMarkPos >= motoMarkLen):
        nowMarkPos -= motoMarkLen
        i += 1
    k += pos

    if (mark[0] == mark[1]):
        # 为原文, 不需要注音
        res.append(f"{{\\kf{kfToken.kf}}}{kfToken.str}")
        j += 1
    else:
        # 需要处理注音
        # 把 kfToken.kf 分给 mark[1]
        jpMarkLen = len(mark[1])
        kfLen = kfToken.kf // jpMarkLen
        res.append(f"{{\\kf{kfLen * jpMarkLen - kfToken.kf}}}{kfToken.str}|<{mark[1][0]}")
        # 倒序看, 这样可以匹配到更多相同的
        tmpList = []
        m0 = motoMarkLen - 1
        m1 = jpMarkLen - 1
        flags: bool = True
        while m1 > 0:
            if (flags and mark[0][m0] == mark[1][m1]):
                tmpList.append(f"{{\\kf{kfLen}}}{mark[1][m1]}")
                m0 -= 1
            else:
                flags = False
                tmpList.append(f"{{\\kf{kfLen}}}#{mark[1][m1]}")
            m1 -= 1
        tmpList.reverse()
        res.extend(tmpList)
        i += 1
        nowMarkPos = 0
        j += motoMarkLen

print("".join(res))

# @todo 不对