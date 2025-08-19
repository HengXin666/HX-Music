import re
from typing import List, Tuple

from src.mark.jpMark import JpMark

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

assLine = r"{\kf20}よおこそ実力至上教室へ{\kf20}is {\kf20}good {\kf16}ラ{\kf13}ス{\kf15}ト{\kf11}チ{\kf11}ャ{\kf11}ン{\kf20}ス{\kf20}に{\kf17}飢{\kf20}え{\kf20}た{\kf22}つ{\kf20}ま{\kf39}先{\kf21}が{\kf21}繰{\kf21}り{\kf21}返{\kf21}す{\kf20}学{\kf20}校{\kf20}生{\kf20}活{\kf20}行{\kf20}か{\kf20}な{\kf20}け{\kf20}れ{\kf20}ば"

def todo(assLine: str):
    mark = JpMark()
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

            # pos = len(kfToken.str)
            while (motoMarkLen > 0):
                j += 1
                motoMarkLen -= pos
                pos = len(kfToken.str)

    return "".join(res)

class _MatchPronunciation:
    @staticmethod
    def isKanji(c):
        # 检查字符是否为汉字(CJK统一表意文字范围)
        return '\u4e00' <= c <= '\u9fff'

    @staticmethod
    def isKana(c):
        # 检查字符是否为假名(平假名或片假名范围)
        return ('\u3040' <= c <= '\u309f') or ('\u30a0' <= c <= '\u30ff')

    @staticmethod
    def splitIntoBlocks(s):
        if not s:
            return []
        blocks = []
        start = 0
        # 遍历字符串, 根据汉字和假名的转换点分块
        for i in range(1, len(s)):
            if _MatchPronunciation.isKanji(s[i]) and _MatchPronunciation.isKana(s[i-1]):
                blocks.append(s[start:i])
                start = i
        blocks.append(s[start:])
        return blocks

    @staticmethod
    def matchPronunciation(pairs):
        res = []
        for s1, s2 in pairs:
            blocks = _MatchPronunciation.splitIntoBlocks(s1)
            j = 0  # s2中的当前位置
            currentPairs = []

            # 处理除最后一个块外的所有块
            for block in blocks[:-1]:
                if not block:
                    continue
                # 取块中最后一个字符(应为假名)
                lastChar = block[-1]
                # 在s2中查找匹配的假名
                k = s2.find(lastChar, j)
                if k == -1:
                    # 未找到匹配, 使用剩余部分并终止
                    k = len(s2) - 1
                # 提取对应的假名部分
                pron = s2[j:k+1]
                currentPairs.append((block, pron))
                j = k + 1

            # 处理最后一个块
            lastBlock = blocks[-1]
            lastPron = s2[j:]
            currentPairs.append((lastBlock, lastPron))

            res.extend(currentPairs)
        return res

class _AssMark:
    @staticmethod
    def markKanJi(kfLine: KfToken, mark: Tuple[str, str]) -> str:
        """注音, 返回注音的assStr
           内部会平均分时间

        Args:
            kfLine (KfToken): 一个Ass播放Token
            mark (Tuple[str, str]): 注音对照

        Returns:
            str: 注音的assStr
        """
        kaNaLen = len(mark[1])            # 全部注音的长度
        avgKfTime = kfLine.kf // kaNaLen  # 平均分时间, 下取整
        # 倒序看, 这样可以匹配到更多相同的
        res: List[str] = []
        m0 = len(mark[0]) - 1             # 原长度
        m1 = kaNaLen - 1                  # 假名长度
        kanJiRight: int = -1              # 计算汉字的右边界, 之后需要使用
        while m1 > 0:
            if (kanJiRight == -1 and mark[0][m0] == mark[1][m1]):
                # 尾部如果是假名, 那么不是注音
                res.append(f"{{\\kf{avgKfTime}}}{mark[1][m1]}")
                m0 -= 1
            else:
                # 不相同, 说明当前已经是汉字注音了, 前面的都是
                kanJiRight = m0
                res.append(f"{{\\kf{avgKfTime}}}#{mark[1][m1]}")
            m1 -= 1
        # 计算 余下的时间: (平均分时间 * 份数 - 全部时间) + 平均分时间
        modTime: int = (avgKfTime * kaNaLen - kfLine.kf) + avgKfTime
        res.append(f"{{\\kf{modTime}}}{mark[0][0:m0+1]}|<{mark[1][0]}")
        res.reverse()
        return "".join(res)

    @staticmethod
    def beikin(kfList: List[KfToken], markList: List[Tuple[str, str]]) -> List[str]:
        res: List[str] = []
        print(kfList, "->", markList)
        kfLen = len(kfList)
        markLen = len(markList)
        if (kfLen == 1 and markLen > 1):
            # 有可能 一个 kfToken.str 对应 多个 mark, 此时应该平均分 k, 给 mark (非逐字)
            # 如: |実力至上　=> [(実力, じつりょく), (至上, しじょう)]

            pass
        elif (kfLen == 1 and markLen == 1):
            # 也可能 一个 kfToken.str 对应 一个 mark, 判断是否注音, 给 合并 add
            # 如: |私 => (私, わたし)
            kfToken = kfList[0]
            mark = markList[0]
            if (mark[0] == mark[1]):
                # 不需要注音
                res.append(f"{{\\kf{kfToken.kf}}}{kfToken.str}")
            else:
                # 需要注音
                res.append(_AssMark.markKanJi(kfToken, mark))
        elif (kfLen > 1 and markLen == 1):
            # 还可能 多个 kfToken.str 对应 一个 mark, 判断是否注音, 偏移 k + add
            # 如: |よ|お|こ|そ => (よおこそ, よおこそ)
            # 如: |飢|え|た 　 => (飢えた, まえた)
            # 从尾部开始, 然后向前, 以保证注音
            mark = markList[0]
            if (mark[0] == mark[1]):
                # 不需要注音
                for it in kfList:
                    res.append(f"{{\\kf{it.kf}}}{it.str}")
            else:
                # 需要注音
                # 注意: 繰り返す, 还需要预处理为 ['繰り', '返す']
                # 然后, 把他们进行二次对齐
                newMarkList = _MatchPronunciation.matchPronunciation(markList)
                if (newMarkList != markList):
                    res.append(_AssMark.work("".join([_.str for _ in kfList]), newMarkList, kfList))
                else:
                    # 如: |飢|え|た 　 => (飢えた, まえた)
                    # 如: |学|校|生|活 => ('学校生活', 'がっこうせいかつ')
                    # 给定模式为 汉字[假名], 假名是可选的, len(汉字) >= 1
                    # 从后往前
                    mIdx = len(mark[0]) - 1
                    kfIdx = kfLen - 1
                    while kfIdx >= 0:
                        kfToken = kfList[kfIdx]
                        if (mark[1][mIdx] == kfToken.str):
                            # 相同, 即假名; 不需要注音
                            res.append(f"{{\\kf{kfToken.kf}}}{kfToken.str}")
                            mIdx -= 1
                            kfIdx -= 1
                        else:
                            # 不相同, 表示开始为注音模式
                            # [0, kfIdx] 都是一个汉字词的, 并且读音难分开
                            kfList = kfList[0:kfIdx+2]
                            kfLink: KfToken = KfToken(
                                sum([_.kf for _ in kfList]),
                                "".join([_.str for _ in kfList])
                            )
                            res.append(_AssMark.markKanJi(kfLink, mark))
                            break
        else:
            # 还可能 多个 kfToken.str 对应 多个 mark, 数据ub!
            print("非法:", kfList, "->", markList)
            pass
        return res

    @staticmethod
    def work(lineStr: str, markList: List[Tuple[str, str]], kfTokenList: List[KfToken]) -> str:
        res: List[str] = []

        i: int = 0
        j: int = 0
        k: int = 0
        n: int = len(lineStr)
        # 按照 i 进行主替换, j 是对齐使用 (带 k 帧)
        while k < n:
            # 有可能 一个 kfToken.str 对应 多个 mark, 此时应该平均分 k, 给 mark (非逐字)
            # 如: |実力至上　=> [(実力, じつりょく), (至上, しじょう)]
            #
            # 也可能 一个 kfToken.str 对应 一个 mark, 判断是否注音, 给 合并 add
            # 如: |私 => (私, わたし)
            #
            # 还可能 多个 kfToken.str 对应 一个 mark, 判断是否注音, 偏移 k + add
            # 如: |よ|お|こ|そ => (よおこそ, よおこそ)
            # 如: |飢|え|た 　 => (飢えた, まえた)
            #
            # 还可能 多个 kfToken.str 对应 多个 mark, 数据ub! 那应该是 情况一

            # 他们都是整的, 也就是不整不行
            eRaBuKfList: List[KfToken] = []
            eRaBuMarkList: List[Tuple[str, str]] = []

            baRanSu: int = 0 # バランス
            flag: bool = True
            while True:
                if (flag or baRanSu > 0):
                    mark = markList[i]
                    eRaBuMarkList.append(mark)
                    baRanSu -= len(mark[0])
                    i += 1
                elif (baRanSu < 0):
                    kf = kfTokenList[j]
                    eRaBuKfList.append(kf)
                    kfLen = len(kf.str)
                    k += kfLen
                    baRanSu += kfLen
                    j += 1
                else: # == 0
                    res.extend(_AssMark.beikin(eRaBuKfList, eRaBuMarkList))
                    break
                flag = False

        return "".join(res)

def fk(assLine: str) -> str:
    mark = JpMark()
    # 期望输入是逐字歌词
    lineStr: str = toLinkStr(assLine)
    markList: List[Tuple[str, str]] = mark.convert(lineStr)    # 飢えた | つま | 先 | が
    kfTokenList: List[KfToken] = parseKfLine(assLine)  # 飢 | え | た | つ | ま | 先 | が

    print("[markList]:", markList)
    print()
    print("[kfTokenList]:", kfTokenList)
    print()
    print("==============================\n")
    return _AssMark.work(lineStr, markList, kfTokenList)
    

print(fk(assLine))