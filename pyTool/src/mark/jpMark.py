import sys
import re
import json
import pykakasi
from typing import List, Tuple
from pathlib import Path
from g2p_en import G2p  # 音素

# 迁移位置
sys.path.append(str(Path(__file__).resolve().parents[2]))
from libs.e2k.src.e2k import C2K, P2K

from ..utils.basePath import BasePath

class JpMark:
    def __init__(self) -> None:
        # 初始化汉字转换
        self._kks = pykakasi.kakasi()

        # 初始化英语转换模型
        self._c2k = C2K()  # 字符到片假名
        self._p2k = P2K()  # 音素到片假名
        self._g2p = G2p()  # 英文->音素

        # 加载本地 英语 -> 片假名 词典
        katakana_dict_path = BasePath.relativePath("./libs/e2k/vendor/katakana_dict.jsonl")
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
        pattern = r'[A-Za-z]+|[ぁ-んァ-ン一-龯]+|[　 、。!?]'
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
        
    def convert(self, text: str) -> List[Tuple[str, str]]:
        """解析

        Args:
            text (str): 句子
        """
        tokens = JpMark.splitMixedText(text)
        convertRes = []
        for token in tokens:
            convertRes.extend(self._kks.convert(token))
        res = []
        for item in convertRes:
            kotoba = item['orig']
            if (self._isEnglishSimple(kotoba)):
                res.append((kotoba, self.englishToKatakana(kotoba)))
            elif (kotoba == item['kana']):
                res.append((kotoba, kotoba))
            else:
                res.append((kotoba, item['hira']))
        return res