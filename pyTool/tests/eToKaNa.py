import include

import json
from pathlib import Path
from g2p_en import G2p  # any G2P library, CMUDict

from libs.e2k.src.e2k import C2K, P2K

# 加载字典, 如果你报错, 请保证自己的工作目录是 ./.., 即 pyTool 而不是 pyTool/tests
katakana_dict_path = Path("./libs/e2k/vendor/katakana_dict.jsonl")
katakana_dict = {}
with open(katakana_dict_path, "r", encoding="utf-8") as f:
    for line in f:
        entry = json.loads(line)
        # 英文转小写作为 key
        katakana_dict[entry["word"].lower()] = entry["kata"]

# 初始化模型
c2k = C2K()  # 字符到片假名
p2k = P2K()  # 音素到片假名
g2p = G2p()  # 英文->音素

def english_to_katakana(word: str, use_phoneme: bool = True) -> str:
    """优先查字典, 查不到用模型"""
    key = word.lower()
    if key in katakana_dict:
        val = katakana_dict[key]
        # 如果是列表, 取第一个
        if isinstance(val, list):
            return val[0]
        return val
    if use_phoneme:
        phonemes = g2p(word)
        return p2k(phonemes)
    else:
        return c2k(word)

# 示例
words = ["computer", "music", "voice", "McDonald's", "ChatGPT", "Heng_Xin", "LoLi", "sister"]
for w in words:
    print(w, "->", english_to_katakana(w))