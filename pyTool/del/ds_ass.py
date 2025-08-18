#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ass_ruby_fixed_final.py
- 注音行在上, 正文在下
- karaoke 行按每个 {\\kN} 音节插入注音, 注音行只显示读音
- 普通行生成注音行(上, 只显示读音) + 原文行(下)
- 优先使用 pykakasi, 回退到 kakasi 命令
- 保留所有 {..} 标签 (不拆分)
作者: DeepSeek
日期: 2023-12-01
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

# ---------------- 分割保留标签 ----------------
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

# ---------------- 字母转片假名 ----------------
def alphaToKana(alpha: str) -> str:
    return "".join(ALPHA_TO_KANA.get(ch, ch) for ch in alpha)

# ---------------- 卡拉OK音节注音 ----------------
# 正则: 捕获 {\\kN} 及其之后到下一个 { 之前的内容
_reK = re.compile(r'\{\\[kK](\d+)\}((?:\{[^}]*\}|[^\\{])*)')

def stripInlineTags(s: str) -> str:
    return re.sub(r'\{[^}]+\}', '', s)

def buildAnnotationForSyllable(sylRaw: str, rubyScale: float) -> str:
    """
    构建单个音节的注音标签
    """
    plain = stripInlineTags(sylRaw)
    # 如果已经是假名，直接隐藏
    if not plain or containsOnlyKana(plain):
        return "{\\alpha&HFF&}" + sylRaw + "{\\r}"
    
    # 含汉字 -> 平假名
    if any(isJapaneseKanji(ch) for ch in plain):
        hira = getHiragana(plain)
        if hira:
            return "{\\alpha&HFF&}" + sylRaw + "{\\alpha&H00&\\fscx" + str(int(rubyScale*100)) + "\\fscy" + str(int(rubyScale*100)) + "}" + hira + "{\\r}"
    
    # 含英文字母 -> 片假名
    if any(isEnglishAlphabet(ch) for ch in plain):
        kana = alphaToKana(plain)
        return "{\\alpha&HFF&}" + sylRaw + "{\\alpha&H00&\\fscx" + str(int(rubyScale*100)) + "\\fscy" + str(int(rubyScale*100)) + "}" + kana + "{\\r}"
    
    # 其他情况
    return "{\\alpha&HFF&}" + sylRaw + "{\\r}"

def processKaraokeTextToAnnotation(textField: str, rubyScale: float) -> str:
    """
    处理卡拉OK文本为注音行
    """
    parts: List[str] = []
    last = 0
    for m in _reK.finditer(textField):
        if m.start() > last:
            parts.append(textField[last:m.start()])
        dur = m.group(1)
        sylRaw = m.group(2)
        ann = "{\\k" + dur + "}" + buildAnnotationForSyllable(sylRaw, rubyScale)
        parts.append(ann)
        last = m.end()
    if last < len(textField):
        parts.append(textField[last:])
    return "".join(parts)

# ---------------- 普通行处理 ----------------
def processPlainSegment(segment: str, rubyScale: float) -> Tuple[str, str]:
    """
    处理普通文本段
    返回 (注音段, 原文本段)
    """
    if not segment:
        return "", ""
    
    annParts: List[str] = []
    plainParts: List[str] = []
    i = 0
    n = len(segment)
    
    while i < n:
        ch = segment[i]
        # 汉字处理
        if isJapaneseKanji(ch):
            j = i + 1
            while j < n and isJapaneseKanji(segment[j]):
                j += 1
            group = segment[i:j]
            hira = getHiragana(group)
            if hira:
                annParts.append("{\\alpha&HFF&}" + group + "{\\alpha&H00&\\fscx" + str(int(rubyScale*100)) + "\\fscy" + str(int(rubyScale*100)) + "}" + hira + "{\\r}")
            else:
                annParts.append("{\\alpha&HFF&}" + group + "{\\r}")
            plainParts.append(group)
            i = j
            continue
        
        # 英文字母处理
        if isEnglishAlphabet(ch):
            j = i + 1
            while j < n and isEnglishAlphabet(segment[j]):
                j += 1
            group = segment[i:j]
            kana = alphaToKana(group)
            annParts.append("{\\alpha&HFF&}" + group + "{\\alpha&H00&\\fscx" + str(int(rubyScale*100)) + "\\fscy" + str(int(rubyScale*100)) + "}" + kana + "{\\r}")
            plainParts.append(group)
            i = j
            continue
        
        # 假名/标点: 注音行隐藏
        if isKana(ch) or ch.isspace() or ch in "ー・、。！？.,!?()-[]〜〜":
            annParts.append("{\\alpha&HFF&}" + ch + "{\\r}")
            plainParts.append(ch)
            i += 1
            continue
        
        # 其他字符
        annParts.append(ch)
        plainParts.append(ch)
        i += 1
    
    return "".join(annParts), "".join(plainParts)

def processTextToAnnotationAndPlain(textField: str, rubyScale: float) -> Tuple[str, str]:
    parts = splitPreservingTags(textField)
    annOut: List[str] = []
    plainOut: List[str] = []
    
    for seg, isTag in parts:
        if isTag:
            # 标签在两行都保留
            annOut.append(seg)
            plainOut.append(seg)
        else:
            a, p = processPlainSegment(seg, rubyScale)
            annOut.append(a)
            plainOut.append(p)
    
    return "".join(annOut), "".join(plainOut)

# ---------------- ASS文件处理 ----------------
def processAssFile(inputPath: str, outputPath: str, rubyScale: float = 0.7) -> None:
    with open(inputPath, 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
    
    outLines: List[str] = []
    ruby_style_created = False
    
    for raw in lines:
        line = raw.rstrip('\n').rstrip('\r')
        leftSpaces = line[:len(line) - len(line.lstrip())]
        stripped = line.lstrip()
        
        # 样式定义行：添加注音样式
        if stripped.startswith('Style:'):
            # 分割完整样式行
            parts = stripped.split(',')
            
            # 确保样式行有足够字段
            if len(parts) < 10:
                outLines.append(raw)
                continue
                
            # 创建注音样式（只创建一次）
            if not ruby_style_created:
                # 创建新样式行
                ruby_style = "Style:Ruby,"
                ruby_style += ",".join(parts[1:])
                
                # 添加到输出
                outLines.append(leftSpaces + ruby_style + '\n')
                ruby_style_created = True
            
            # 保留原样式行
            outLines.append(raw)
            continue
        
        # 对话行处理
        if stripped.startswith('Dialogue:'):
            parts = stripped.split(',', 9)
            if len(parts) < 10:
                outLines.append(raw)
                continue
                
            textField = parts[9]
            originalStyle = parts[3]  # 原始样式
            
            # 卡拉OK行处理
            if re.search(r'\{\\[kK]\d+\}', textField):
                # 注音行（上）
                annText = processKaraokeTextToAnnotation(textField, rubyScale)
                partsAnn = parts.copy()
                partsAnn[3] = "Ruby"  # 使用注音样式
                partsAnn[9] = annText
                
                # 原文行（下）
                partsPlain = parts.copy()
                partsPlain[9] = textField
                
                # 注音行在上，正文行在下
                outLines.append(leftSpaces + ','.join(partsAnn) + '\n')
                outLines.append(leftSpaces + ','.join(partsPlain) + '\n')
                continue
            
            # 普通行处理
            else:
                annText, plainText = processTextToAnnotationAndPlain(textField, rubyScale)
                
                # 注音行（上）
                partsAnn = parts.copy()
                partsAnn[3] = "Ruby"  # 使用注音样式
                partsAnn[9] = annText
                
                # 原文行（下）
                partsPlain = parts.copy()
                partsPlain[9] = plainText
                
                # 注音行在上，正文行在下
                outLines.append(leftSpaces + ','.join(partsAnn) + '\n')
                outLines.append(leftSpaces + ','.join(partsPlain) + '\n')
                continue
        
        # 非对话行直接保留
        outLines.append(raw)
    
    # 如果还没有创建注音样式，添加一个基本样式
    if not ruby_style_created:
        ruby_style = "Style:Ruby,Microsoft YaHei,24,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,0,2,10,10,10,0\n"
        outLines.insert(0, ruby_style)
    
    with open(outputPath, 'w', encoding='utf-8-sig') as f:
        f.writelines(outLines)

# ---------------- 命令行接口 ----------------
def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description='ASS注音工具: 注音行在上，正文行在下')
    parser.add_argument('input', help='输入ASS文件')
    parser.add_argument('output', help='输出ASS文件')
    parser.add_argument('--ruby-scale', type=float, default=0.7, 
                        help='注音缩放比例 (0.0-1.0, 默认0.7)')
    args = parser.parse_args(argv)
    processAssFile(args.input, args.output, args.ruby_scale)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())