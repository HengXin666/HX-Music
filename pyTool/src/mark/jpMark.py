import sys
import re
import json
import pykakasi
from typing import List, Tuple
from pathlib import Path
from g2p_en import G2p  # 音素

from e2k import C2K, P2K

from ..utils.basePath import BasePath

# 映射表, hock 一些非日语的表示
SYMBOL_MAP = {
    '!': '!',
    '?': '?',
    '_': '_',
    '-': '-',
    '.': '.',
    '\'': '\'',
}

CN_TO_JP_MAP = {
    "编曲": "編曲",   # 编曲 -> 日语汉字“編曲”
    "调音": "調音",   # 调音 -> 日语汉字“調音”
    "作曲": "作曲",
    "作词": "作詞",
    "制作人": "プロデューサー",  # 如果需要用片假名
    # 可以继续扩展更多
}

class JpMark:
    def __init__(self) -> None:
        # 初始化汉字转换
        self._kks = pykakasi.kakasi()

        # 初始化英语转换模型
        self._c2k = C2K()  # 字符到片假名
        self._p2k = P2K()  # 音素到片假名
        self._g2p = G2p()  # 英文->音素

        # 加载本地 英语 -> 片假名 词典
        katakana_dict_path = BasePath.relativePath("src/data/katakana_dict.jsonl")
        self._katakanaDict = {}
        with open(katakana_dict_path, "r", encoding="utf-8") as f:
            for line in f:
                entry = json.loads(line)
                # 英文转小写作为 key
                self._katakanaDict[entry["word"].lower()] = entry["kata"]

    def _isEnglishSimple(self, s: str) -> bool:
        # 如果 G2P 能生成音素序列，则大概率是英语
        try:
            phonemes = self._g2p(s)
            return bool(phonemes)
        except:
            return False
    
    @staticmethod
    def splitMixedText(text: str) -> list[str]:
        # 日文字符作为单独的 token, 英文单词按空格拆分
        pattern = (
            r'[A-Za-z]+'                          # 英文单词
            r'|[ぁ-んァ-ンヴヵヶヷ-ヺーｦ-ﾟ一-龯]+'     # 日文平假名+片假名+汉字
            r'|[\u3000-\u303F\u3040-\u309F\u30A0-\u30FF\u4E00-\u9FAF\u31F0-\u31FF]+'
            r'|[0-9]+'                             # 数字
            r'|[\\/,\.\?!！？、。；;:：…〜～\'\uFF00-\uFFEF]'      # 常见标点(全角+半角)
            r'|[♪♫♬♩♭♯☆★♥❤✨〽※]'              # 歌词符号/特殊装饰
            r'|[\-\_—\+＝=／／‖｜|]'                 # 连字符/斜杠/竖线等
            r'|[「」『』【】（）\(\)\[\]{}<>〈〉《》]'  # 各类括号
            r'|[】\*\.\&\^\%\$\#@~`"]'               # 额外符号：* . & ^ % $ # @ ~ ` "
            r'|[ 　]'                                 # 空格
        )
        return re.findall(pattern, text)

    def englishToKatakana(self, word: str, usePhoneme: bool = True) -> str:
        """优先查字典, 查不到用模型"""
        key = word.lower()
        if key in self._katakanaDict:
            val = self._katakanaDict[key]
            # 如果是列表, 取第一个
            if isinstance(val, list):
                return val[0]
            return val
        if usePhoneme:
            phonemes = self._g2p(word)
            return self._p2k(phonemes)
        else:
            return self._c2k(word)

    def safeConvert(self, token):
        pattern = r'[ぁ-んァ-ンヴヵヶヷ-ヺー一-龯]+'  # 日文字符
        # 如果是日文字符, 用 kks.convert
        if re.match(pattern, token):
            return self._kks.convert(token)
        # 否则原样返回
        return [{'orig': token, 'hira': token, 'kana': token, 'hepburn': token}]

    @staticmethod
    def _preprocessing(text: str) -> str:
        """
        如果 text 是映射表里的词, 直接返回对应日语
        否则原样返回
        """
        return CN_TO_JP_MAP.get(text, text)
    
    @staticmethod
    def _fixReadings(tokens: list[dict]) -> list[dict]:
        """
        特化处理, 一些分割、注音不正确的, 常见的, 可以手动排除
        """
        if len(tokens) == 0:
            return []
        fixed = [tokens[0]]
        i = 1
        while i < len(tokens):
            # 检查: あの + 日
            if tokens[i - 1]["orig"] == "あの" and tokens[i]["orig"] == "日":
                fixed.pop()
                fixed.append({
                    "orig": "あの日",
                    "hira": "あのひ",
                    "kana": "アノヒ",
                    "hepburn": "ano hi",
                    "kunrei": "ano hi",
                    "passport": "ano hi"
                })
            else:
                fixed.append(tokens[i])
            i += 1
        return fixed

    def convert(self, text: str) -> List[Tuple[str, str]]:
        """解析

        Args:
            text (str): 句子
        """
        tokens = JpMark.splitMixedText(text)
        convertRes = []
        for token in tokens:
            convertRes.extend(
                JpMark._fixReadings(
                    self._kks.convert(
                        JpMark._preprocessing(token)
            )))
        res = []
        for item in convertRes:
            kotoba = item['orig']
            if (self._isEnglishSimple(kotoba)):
                res.append((kotoba, self.englishToKatakana(kotoba)))
            elif (kotoba == item['kana']):
                res.append((kotoba, kotoba))
            else:
                res.append((kotoba, item['hira']))
            if (res[-1][0] in SYMBOL_MAP):
                res[-1] = (res[-1][0], SYMBOL_MAP[res[-1][0]])
            elif (res[-1][1] == ''):
                res[-1] = (res[-1][0], res[-1][0])
        return res