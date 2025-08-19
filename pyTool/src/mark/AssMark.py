import re
from typing import List, Tuple

from .jpMark import JpMark

class KfToken:
    def __init__(self, kf: int = 0, text: str = "", type: str = "\\kf") -> None:
        self.kf: int = kf
        self.str: str = text
        self.type: str = type

    def __repr__(self) -> str:
        return f"KfToken(kf={self.kf}, str='{self.str}', type='{self.type}')"

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

class AssMark:
    @staticmethod
    def _distributeEvenly(total: int, n: int) -> List[int]:
        """
        把 total 平均分成 n 份,尽可能均匀,返回一个 list
        例如: total=5, n=3 -> [2,2,1]
            total=10, n=3 -> [4,3,3]
        """
        base = total // n
        remainder = total % n
        res = []
        for i in range(n):
            # 前 remainder 个元素多分 1
            if i < remainder:
                res.append(base + 1)
            else:
                res.append(base)
        return res

    @staticmethod
    def _markKanJi(kfLine: KfToken, mark: Tuple[str, str]) -> str:
        """注音, 返回注音的assStr
           内部会平均分时间

        Args:
            kfLine (KfToken): 一个Ass播放Token
            mark (Tuple[str, str]): 注音对照

        Returns:
            str: 注音的assStr
        """
        kaNaLen = len(mark[1])            # 全部注音的长度
        times = AssMark._distributeEvenly(kfLine.kf, len(mark[1]))
        timesIdx = 0
        # 倒序看, 这样可以匹配到更多相同的
        res: List[str] = []
        m0 = len(mark[0]) - 1             # 原长度
        m1 = kaNaLen - 1                  # 假名长度
        kanJiRight: int = -1              # 计算汉字的右边界, 之后需要使用
        while m1 > 0:
            if (kanJiRight == -1 and mark[0][m0] == mark[1][m1]):
                # 尾部如果是假名, 那么不是注音
                res.append(f"{{{kfLine.type}{times[timesIdx]}}}{mark[1][m1]}")
                m0 -= 1
            else:
                # 不相同, 说明当前已经是汉字注音了, 前面的都是
                kanJiRight = m0
                res.append(f"{{{kfLine.type}{times[timesIdx]}}}#|<{mark[1][m1]}")
            timesIdx += 1
            m1 -= 1
        # 计算 余下的时间: (平均分时间 * 份数 - 全部时间) + 平均分时间
        res.append(f"{{{kfLine.type}{times[timesIdx]}}}{mark[0][0:m0+1]}|<{mark[1][0]}")
        res.reverse()
        return "".join(res)

    @staticmethod
    def _markLine(kfList: List[KfToken], markList: List[Tuple[str, str]]) -> List[str]:
        """把传入的内容进行注音

        Args:
            kfList (List[KfToken]): k帧
            markList (List[Tuple[str, str]]): 注音数据

        Returns:
            List[str]: 注音assStrList
        """
        res: List[str] = []
        kfLen = len(kfList)
        markLen = len(markList)
        if (kfLen == 1 and markLen > 1):
            # 有可能 一个 kfToken.str 对应 多个 mark, 此时应该平均分 k, 给 mark (非逐字)
            # 如: |実力至上　=> [(実力, じつりょく), (至上, しじょう)]
            kfToken = kfList[0]
            times = AssMark._distributeEvenly(kfToken.kf, markLen) # 均分时间
            timesIdx = 1
            kfList = [KfToken(times[1], markList[0][0])]
            for i in range(1, markLen):
                kfList.append(KfToken(times[timesIdx], markList[i][0]))
                timesIdx += 1
            res.extend(AssMark._doMark("".join([_.str for _ in kfList]), markList, kfList))
        elif (kfLen == 1 and markLen == 1):
            # 也可能 一个 kfToken.str 对应 一个 mark, 判断是否注音, 给 合并 add
            # 如: |私 => (私, わたし)
            kfToken = kfList[0]
            mark = markList[0]
            if (mark[0] == mark[1]):
                # 不需要注音
                res.append(f"{{{kfToken.type}{kfToken.kf}}}{kfToken.str}")
            else:
                # 需要注音
                res.append(AssMark._markKanJi(kfToken, mark))
        elif (kfLen > 1 and markLen == 1):
            # 还可能 多个 kfToken.str 对应 一个 mark, 判断是否注音, 偏移 k + add
            # 如: |よ|お|こ|そ => (よおこそ, よおこそ)
            # 如: |飢|え|た 　 => (飢えた, まえた)
            # 从尾部开始, 然后向前, 以保证注音
            mark = markList[0]
            if (mark[0] == mark[1]):
                # 不需要注音
                for it in kfList:
                    res.append(f"{{{it.type}{it.kf}}}{it.str}")
            else:
                # 需要注音
                # 注意: 繰り返す, 还需要预处理为 ['繰り', '返す']
                # 然后, 把他们进行二次对齐
                newMarkList = _MatchPronunciation.matchPronunciation(markList)
                if (newMarkList != markList):
                    res.append(AssMark._doMark("".join([_.str for _ in kfList]), newMarkList, kfList))
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
                            res.append(f"{{{kfToken.type}{kfToken.kf}}}{kfToken.str}")
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
                            res.append(AssMark._markKanJi(kfLink, mark))
                            break
        else:
            # 还可能 多个 kfToken.str 对应 多个 mark, 数据ub!
            print("非法:", kfList, "->", markList)
        return res

    @staticmethod
    def _doMark(lineStr: str, markList: List[Tuple[str, str]], kfTokenList: List[KfToken]) -> str:
        """进行注音

        Args:
            lineStr (str): 需要注音的文本
            markList (List[Tuple[str, str]]): _description_
            kfTokenList (List[KfToken]): _description_

        Returns:
            str: 带注音Ass
        """
        res: List[str] = []

        print(lineStr)
        print(markList)
        print(kfTokenList)
        print()

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
                    res.extend(AssMark._markLine(eRaBuKfList, eRaBuMarkList))
                    break
                flag = False

        return "".join(res)

    @staticmethod
    def _parseKfLine(assLine: str) -> List[KfToken]:
        """
        解析一行 ASS 字幕中的 \\k / \\kf / \\ko 标签,
        返回 KfToken 列表 (保持顺序)
        """
        tokens: List[KfToken] = []
        # 匹配 {\\k\d+}, {\\kf\d+}, {\\ko\d+}
        pattern = r"\{\\(k|kf|ko)(\d+)\}([^\{]+)"
        for match in re.finditer(pattern, assLine):
            kType = "\\" + match.group(1)   # 标签类型, 如 "\kf"
            kf_time = int(match.group(2))   # 持续时间
            text = match.group(3)           # 对应文字
            tokens.append(KfToken(kf_time, text, kType))
        return tokens


    @staticmethod
    def _toLinkStr(assLine: str) -> str:
        """
        提取去掉 k 标签后的纯文字
        """
        pattern = r"\{\\(?:k|kf|ko)\d+\}([^\{]+)"
        return "".join(re.findall(pattern, assLine))

    @staticmethod
    def mark(assLine: str) -> str:
        """将一行仅带k帧的ass内容, 加上日语注音

        Args:
            assLine (str): ass文本 (带k帧)

        Returns:
            str: 带注音的ass k帧内容
        """

        mark = JpMark()
        # 期望输入是k帧率歌词
        lineStr: str = AssMark._toLinkStr(assLine)
        markList: List[Tuple[str, str]] = mark.convert(lineStr)
        kfTokenList: List[KfToken] = AssMark._parseKfLine(assLine)
        return AssMark._doMark(lineStr, markList, kfTokenList)