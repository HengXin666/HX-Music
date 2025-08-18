#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ass_ruby_fixed_final.py
- 注音行在上, 正文在下
- karaoke 行按每个 {\\kN} 音节插入注音, 注音行只显示读音; 若音节已经全是假名则注音行只隐藏原文不重复显示
- 普通行生成注音行(上, 只显示读音) + 原文行(下)
- 优先使用 pykakasi, 回退到 kakasi 命令
- 保留所有 {..} 标签 (不拆分)
作者: ChatGPT (GPT-5 Thinking mini)
日期: 2025-08-18
"""
from __future__ import annotations

import argparse
import re
import shlex
import subprocess
from functools import lru_cache
from typing import List, Tuple, Dict, Optional

# ---------------- 映射表 ----------------
ALPHA_TO_KANA: Dict[str, str] = {
    'A': 'エー', 'B': 'ビー', 'C': 'シー', 'D': 'ディー', 'E': 'イー',
    'F': 'エフ', 'G': 'ジー', 'H': 'エイチ', 'I': 'アイ', 'J': 'ジェー',
    'K': 'ケー', 'L': 'エル', 'M': 'エム', 'N': 'エヌ', 'O': 'オー',
    'P': 'ピー', 'Q': 'キュー', 'R': 'アール', 'S': 'エス', 'T': 'ティー',
    'U': 'ユー', 'V': 'ブイ', 'W': 'ダブリュー', 'X': 'エックス', 'Y': 'ワイ', 'Z': 'ゼット'
}
for k, v in list(ALPHA_TO_KANA.items()):
    ALPHA_TO_KANA[k.lower()] = v

# 可选 Yahoo API 回退 (如需则填入)
YAHOO_APPID = ""  # 留空则不调用 Yahoo

# ---------------- pykakasi 优先 ----------------
try:
    import pykakasi  # type: ignore
    _HAS_PYKAKASI = True
    _PKK = pykakasi.kakasi()
except Exception:
    _HAS_PYKAKASI = False
    _PKK = None

# ---------------- Unicode 判断 ----------------
def isJapaneseKanji(ch: str) -> bool:
    if not ch:
        return False
    cp = ord(ch)
    return (
        0x4E00 <= cp <= 0x9FFF or
        0x3400 <= cp <= 0x4DBF or
        0x20000 <= cp <= 0x2A6DF or
        0x2A700 <= cp <= 0x2B73F or
        0x2B740 <= cp <= 0x2B81F or
        0x2B820 <= cp <= 0x2CEAF
    )

def isEnglishAlphabet(ch: str) -> bool:
    return ('A' <= ch <= 'Z') or ('a' <= ch <= 'z')

def isHiragana(ch: str) -> bool:
    cp = ord(ch)
    return 0x3040 <= cp <= 0x309F

def isKatakana(ch: str) -> bool:
    cp = ord(ch)
    return (0x30A0 <= cp <= 0x30FF) or (0x31F0 <= cp <= 0x31FF) or cp == 0x30FC

def isKana(ch: str) -> bool:
    return isHiragana(ch) or isKatakana(ch)

def containsOnlyKana(s: str) -> bool:
    if not s:
        return False
    for ch in s:
        if ch.isspace():
            continue
        if ch in "ー・、。！？.,!?()-[]〜〜":
            continue
        if not isKana(ch):
            return False
    return True

# ---------------- split preserving {..} tags ----------------
def splitPreservingTags(text: str) -> List[Tuple[str, bool]]:
    """
    返回 [(segment, isTag)].
    isTag=True 表示该段是完整的大括号标签 (包含两侧的 { }).
    """
    out: List[Tuple[str, bool]] = []
    i = 0
    n = len(text)
    while i < n:
        if text[i] == '{':
            j = text.find('}', i+1)
            if j == -1:
                out.append((text[i:], False))
                break
            out.append((text[i:j+1], True))
            i = j + 1
        else:
            j = text.find('{', i)
            if j == -1:
                out.append((text[i:], False))
                break
            out.append((text[i:j], False))
            i = j
    return out

# ---------------- pykakasi / kakasi 封装 ----------------
@lru_cache(maxsize=4096)
def getHiraganaWithPykakasi(text: str) -> str:
    if not _HAS_PYKAKASI or _PKK is None:
        return ""
    try:
        conv = _PKK.convert(text)
        return "".join(item.get('hira', '') for item in conv)
    except Exception:
        return ""

@lru_cache(maxsize=4096)
def getHiraganaWithKakasiCmd(text: str) -> str:
    if not text:
        return ""
    try:
        safe = shlex.quote(text)
        cmd = f"/bin/sh -c \"printf %s {safe} | kakasi -Ja -Ha -Ka -Ea -s\""
        p = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=4)
        if p.returncode == 0:
            return p.stdout.decode('utf-8', errors='ignore').strip()
    except Exception:
        pass
    return ""

def getHiragana(text: str) -> str:
    if not text:
        return ""
    if _HAS_PYKAKASI:
        r = getHiraganaWithPykakasi(text)
        if r:
            return r
    r2 = getHiraganaWithKakasiCmd(text)
    return r2 or ""

# ---------------- alpha -> katakana ----------------
def alphaToKana(alpha: str) -> str:
    return "".join(ALPHA_TO_KANA.get(ch, ch) for ch in alpha)

# ---------------- karaoke 音节注音 (注音在上, 只显示读音) ----------------
# 正则: 捕获 {\\kN} 及其之后到下一个 { 之前的内容, 允许该段包含小标签
_reK = re.compile(r'\{\\[kK](\d+)\}((?:\{[^}]*\}|[^\\{])*)')

def stripInlineTags(s: str) -> str:
    return re.sub(r'\{[^}]+\}', '', s)

def buildAnnotationForSyllable(sylRaw: str, fsPercent: int, upOffset: int) -> str:
    """
    sylRaw: 音节原始内容, 可能包含标签
    返回注音段。注音段结构遵循:
      - 隐藏原文: '{\\alpha&HFF&}' + sylRaw
      - 若需要读音则追加: '{\\alpha&H00&\\fs{fs}%\\fscx100\\fscy100\\up{up}}' + reading + '{\\r}'
      - 若不需要读音则追加 '{\\r}' 结束覆盖
    注: 注释中所有显示的 override 标签都用双反斜杠(例如 '{\\\\k0}') 以符合你的要求.
    """
    plain = stripInlineTags(sylRaw)
    hideOriginal = "{\\alpha&HFF&}" + sylRaw
    # 如果 plain 为空或已经全是假名, 注音行不应重复显示读音, 只隐藏原文
    if not plain or containsOnlyKana(plain):
        return hideOriginal + "{\\r}"
    # 含汉字 -> 平假名
    if any(isJapaneseKanji(ch) for ch in plain):
        hira = getHiragana(plain)
        if hira:
            readingTag = "{\\alpha&H00&\\fs" + str(fsPercent) + "%\\fscx100\\fscy100\\up" + str(upOffset) + "}" + hira + "{\\r}"
            return hideOriginal + readingTag
        else:
            return hideOriginal + "{\\r}"
    # 含英文字母 -> 片假名
    if any(isEnglishAlphabet(ch) for ch in plain):
        kana = alphaToKana(plain)
        readingTag = "{\\alpha&H00&\\fs" + str(fsPercent) + "%\\fscx100\\fscy100\\up" + str(upOffset) + "}" + kana + "{\\r}"
        return hideOriginal + readingTag
    # 其他情况不插入读音
    return hideOriginal + "{\\r}"

def processKaraokeTextToAnnotation(textField: str, rubyScale: float, rubyUp: int) -> str:
    """
    把 karaoke 行转换成注音(上)行。保留非 {\\k} 区域原样, 对每个 {\\kN} 插入隐藏原文+可能的读音。
    """
    fsPercent = max(1, int(rubyScale * 100 + 0.5))
    parts: List[str] = []
    last = 0
    for m in _reK.finditer(textField):
        if m.start() > last:
            parts.append(textField[last:m.start()])
        dur = m.group(1)
        sylRaw = m.group(2)
        ann = "{\\k" + dur + "}" + buildAnnotationForSyllable(sylRaw, fsPercent, rubyUp)
        parts.append(ann)
        last = m.end()
    if last < len(textField):
        parts.append(textField[last:])
    return "".join(parts)

# ---------------- 非 karaoke 行: 注音行(上) + 原文行(下) ----------------
def processPlainSegmentToAnnotationAndPlain(segment: str, rubyScale: float, rubyUp: int) -> Tuple[str, str]:
    """
    segment: 不含标签的纯文本段
    返回 (annotationSegment, plainSegment)
    annotationSegment: 注音行该段 (隐藏原文, 仅显示必要读音)
    plainSegment: 原文段
    """
    if not segment:
        return "", ""
    fsPercent = max(1, int(rubyScale * 100 + 0.5))
    i = 0
    n = len(segment)
    annParts: List[str] = []
    plainParts: List[str] = []
    while i < n:
        ch = segment[i]
        if isJapaneseKanji(ch):
            j = i + 1
            while j < n and isJapaneseKanji(segment[j]):
                j += 1
            group = segment[i:j]
            hira = getHiragana(group)
            if hira:
                annParts.append("{\\alpha&HFF&}" + group + "{\\alpha&H00&\\fs" + str(fsPercent) + "%\\fscx100\\fscy100\\up" + str(rubyUp) + "}" + hira + "{\\r}")
            else:
                annParts.append("{\\alpha&HFF&}" + group + "{\\r}")
            plainParts.append(group)
            i = j
            continue
        if isEnglishAlphabet(ch):
            j = i + 1
            while j < n and isEnglishAlphabet(segment[j]):
                j += 1
            group = segment[i:j]
            kana = alphaToKana(group)
            annParts.append("{\\alpha&HFF&}" + group + "{\\alpha&H00&\\fs" + str(fsPercent) + "%\\fscx100\\fscy100\\up" + str(rubyUp) + "}" + kana + "{\\r}")
            plainParts.append(group)
            i = j
            continue
        # 假名/标点: 注音行隐藏原文但不插入读音
        if isKana(ch) or ch.isspace() or ch in "ー・、。！？.,!?()-[]〜〜":
            annParts.append("{\\alpha&HFF&}" + ch + "{\\r}")
            plainParts.append(ch)
            i += 1
            continue
        # 其它字符原样
        annParts.append(ch)
        plainParts.append(ch)
        i += 1
    return "".join(annParts), "".join(plainParts)

def processTextToAnnotationAndPlain(textField: str, rubyScale: float, rubyUp: int) -> Tuple[str, str]:
    parts = splitPreservingTags(textField)
    annOut: List[str] = []
    plainOut: List[str] = []
    for seg, isTag in parts:
        if isTag:
            # 标签在两行都保留原样
            annOut.append(seg)
            plainOut.append(seg)
        else:
            a, p = processPlainSegmentToAnnotationAndPlain(seg, rubyScale, rubyUp)
            annOut.append(a)
            plainOut.append(p)
    return "".join(annOut), "".join(plainOut)

# ---------------- 读写 ASS ----------------
def processAssFile(inputPath: str, outputPath: str, rubyScale: float = 0.7, rubyUp: int = 30) -> None:
    with open(inputPath, 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
    outLines: List[str] = []
    for raw in lines:
        line = raw.rstrip('\n').rstrip('\r')
        leftSpaces = line[:len(line) - len(line.lstrip())]
        stripped = line.lstrip()
        if stripped.startswith('Dialogue:'):
            parts = stripped.split(',', 9)
            if len(parts) >= 10:
                textField = parts[9]
                if re.search(r'\{\\[kK]\d+\}', textField):
                    annText = processKaraokeTextToAnnotation(textField, rubyScale, rubyUp)
                    partsAnn = parts.copy()
                    partsPlain = parts.copy()
                    partsAnn[9] = annText
                    partsPlain[9] = textField
                    # 注音行在上, 正文行在下
                    outLines.append(leftSpaces + ','.join(partsAnn) + '\n')
                    outLines.append(leftSpaces + ','.join(partsPlain) + '\n')
                    continue
                else:
                    annText, plainText = processTextToAnnotationAndPlain(textField, rubyScale, rubyUp)
                    if annText and annText != plainText:
                        partsAnn = parts.copy()
                        partsPlain = parts.copy()
                        partsAnn[9] = annText
                        partsPlain[9] = plainText
                        outLines.append(leftSpaces + ','.join(partsAnn) + '\n')
                        outLines.append(leftSpaces + ','.join(partsPlain) + '\n')
                        continue
                    else:
                        outLines.append(raw)
                        continue
        outLines.append(raw)
    with open(outputPath, 'w', encoding='utf-8-sig') as f:
        f.writelines(outLines)

# ---------------- CLI ----------------
def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description='ASS 注音修正版: 注音在上, 正文在下, 假名不重复显示')
    parser.add_argument('input', help='输入 .ass 文件')
    parser.add_argument('output', help='输出 .ass 文件')
    parser.add_argument('--ruby-scale', type=float, default=0.7, help='注音缩放 0.0-1.0 (默认 0.7)')
    parser.add_argument('--ruby-up', type=int, default=30, help='注音上移像素 (默认 30)')
    args = parser.parse_args(argv)
    processAssFile(args.input, args.output, args.ruby_scale, args.ruby_up)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
