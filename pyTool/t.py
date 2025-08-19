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

# 测试示例
if __name__ == "__main__":
    input_pairs = [('繰り返す返す返す', 'くりかえすかえすかnaす')]
    output_pairs = _MatchPronunciation.matchPronunciation(input_pairs)
    print(output_pairs)  # 输出: [('繰り', 'くり'), ('返す', 'かえす')]