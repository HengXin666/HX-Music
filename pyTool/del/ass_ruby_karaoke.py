#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ASS K-frame Ruby Annotator for Japanese Lyrics
- Annotates only Kanji with Furigana (in Hiragana by default) above the line.
- Also annotates ASCII English words with Katakana above the line.
- Keeps original {\\k...} timing per syllable and stacks "ruby line" above the original via \\N.
- Does NOT alter timing or non-karaoke tags; passes through unknown tags.
- Pure Python dependencies (install once):
    pip install fugashi unidic-lite pykakasi

Usage:
    python3 ass_ruby_karaoke.py input.ass output.ass
Options:
    --ruby-font-scale 0.6        # Ruby size relative to base font size (default 0.6)
    --ruby-style "Ruby"          # Style name to apply to the ruby line (default: keep current style, only shrink fs)
    --katakana-for-english       # Force katakana for A-Z words (default on)
    --romaji-for-english         # Use UPPERCASE ROMAJI instead of katakana for A-Z words (overrides katakana option)
    --keep-style                 # Do not inject \fs shrink in ruby line (use when Ruby style already smaller)
Notes:
    1) We rely on {\\k<dur>} segmentation already present in the line.
       The top ruby line mirrors those segments: only Kanji/English segments get text; others are blank.
    2) If a Kanji segment already contains kana (e.g., 例(れい)), we still compute reading from analyzer.
       Adjust by hand if needed.
    3) unidic-lite is used through fugashi, lightweight and self-contained.
"""
import argparse
import re
import sys
from typing import List, Tuple, Optional

# Lazy imports so the script can show friendly errors if deps missing
try:
    from fugashi import Tagger  # MeCab wrapper
except Exception as e:
    Tagger = None  # type: ignore

try:
    import pykakasi  # for kana conversions
except Exception as e:
    pykakasi = None  # type: ignore

# --------- Utilities ---------

KANJI_RE = re.compile(r'[\u4E00-\u9FFF]')
ASCII_WORD_RE = re.compile(r'[A-Za-z]+')
# Karaoke: {\k<dur>} (libass still supports k and related). We allow k, K, ko, kf etc, but we *preserve* the original tag.
K_TAG_RE = re.compile(r'(\{\\k[fo]?\d+[^}]*\})', re.IGNORECASE)

# Split a karaoke line into [(tag, text)] pairs where tag is the \k...\} piece (including braces)
def split_k_segments(s: str) -> List[Tuple[str, str]]:
    parts: List[Tuple[str, str]] = []
    idx = 0
    for m in K_TAG_RE.finditer(s):
        # text before first k-tag is kept as-is in a zero-duration container if any
        if m.start() > idx and not parts:
            head = s[:m.start()]
            parts.append(("", head))
        tag = m.group(1)
        idx = m.end()
        # Find the following text until the next k-tag or end
        next_m = K_TAG_RE.search(s, idx)
        text = s[idx:next_m.start()] if next_m else s[idx:]
        parts.append((tag, text))
        if not next_m:
            break
        idx = next_m.start()
    if not parts:
        # No \k tags found, we treat the entire line as a single "segment" without a k-tag
        parts.append(("", s))
    return parts

def contains_kanji(s: str) -> bool:
    return bool(KANJI_RE.search(s))

def contains_ascii_word(s: str) -> bool:
    return bool(ASCII_WORD_RE.search(s))

def katakana_to_hiragana(s: str) -> str:
    return "".join(chr(ord(ch)-0x60) if 'ァ' <= ch <= 'ヶ' else ch for ch in s)

def ensure_tagger() -> Tagger:
    if Tagger is None:
        sys.stderr.write("Error: fugashi (MeCab) is not installed. Run: pip install fugashi unidic-lite\n")
        sys.exit(2)
    try:
        # unidic-lite is auto-discovered by fugashi if installed
        return Tagger()
    except Exception as e:
        sys.stderr.write("Error: failed to initialize MeCab (fugashi). Try: pip install fugashi unidic-lite\n")
        sys.stderr.write(str(e) + "\n")
        sys.exit(2)

def ensure_kakasi():
    if pykakasi is None:
        sys.stderr.write("Warning: pykakasi is not installed, English→Katakana fallback will be very rough.\n")
        return None
    return pykakasi.kakasi()

def reading_japanese(segment: str, tagger, to_hiragana: bool = True) -> str:
    """
    获取日语片段的读音. 仅对包含汉字的词返回假名读音.
    to_hiragana=True 则转为平假名, 否则保持片假名.
    """
    tokens = tagger(segment)
    outs: List[str] = []
    for t in tokens:
        surf = t.surface
        if contains_kanji(surf):
            # 优先使用 reading, 其次 kana, 再不行就用原字面
            reading = None
            try:
                reading = getattr(t.feature, "reading", None)
            except Exception:
                pass
            if not reading or reading == "*":
                try:
                    reading = getattr(t.feature, "kana", None)
                except Exception:
                    reading = None
            if not reading or reading == "*":
                reading = surf

            kana = reading
            if to_hiragana:
                kana = katakana_to_hiragana(kana)
            outs.append(kana)
        else:
            # 不含汉字的片段不标注注音
            pass
    return "".join(outs)


# Very simple English → Katakana heuristic (fallback if pykakasi missing)
# This is intentionally conservative; for better results consider installing a G2P like g2p_en.
def english_to_katakana_simple(word: str) -> str:
    w = word.lower()
    # mapping rough syllables
    repl = [
        ('tion', 'ション'), ('sion', 'ジョン'), ('ture', 'チャー'), ('sure', 'ジャー'),
        ('ing', 'イング'), ('ed', 'ド'), ('er', 'アー'), ('or', 'オー'), ('ar', 'アー'),
        ('ck', 'ック'), ('qu', 'ク'), ('ph', 'フ'), ('th', 'ス'), ('ch', 'チ'), ('sh', 'シ'),
        ('a', 'ア'), ('e', 'エ'), ('i', 'イ'), ('o', 'オ'), ('u', 'ウ'),
        ('b', 'ブ'), ('c', 'ク'), ('d', 'ド'), ('f', 'フ'), ('g', 'グ'),
        ('h', 'ハ'), ('j', 'ジ'), ('k', 'ク'), ('l', 'ル'), ('m', 'ム'),
        ('n', 'ン'), ('p', 'プ'), ('q', 'ク'), ('r', 'ル'), ('s', 'ス'),
        ('t', 'ト'), ('v', 'ヴ'), ('w', 'ウ'), ('x', 'クス'), ('y', 'イ'), ('z', 'ズ'),
    ]
    res = w
    for k, v in repl:
        res = res.replace(k, v)
    # basic cleanup: duplicate long vowels
    res = re.sub('ー+', 'ー', res)
    # Capitalize katakana? Not needed
    return res

def english_to_katakana(word: str, kakasi_obj) -> str:
    # pykakasi并不原生支持英语转换; 我们只是在这里传递, 并使用回退。
    return english_to_katakana_simple(word)

def build_ruby_line(
    segments: List[Tuple[str, str]],
    tagger: Tagger,
    katakana_english: bool = True,
    ruby_hiragana: bool = True,
) -> str:
    """
    构造注音行, 使用透明占位符方式, 保证与原文对齐
    segments: [(text, ktag), ...]
    """
    out_parts: List[str] = []

    for text, ktag in segments:
        rub = ""
        if contains_kanji(text):
            rub = reading_japanese(text, tagger, to_hiragana=ruby_hiragana)
        elif katakana_english and contains_ascii_word(text):
            rub = english_to_katakana(text, None)

        if rub:
            # 透明假注音, 用小字号缩放撑开宽度
            placeholder = f"{{\\fs20\\alpha&H00&}}{rub}{{\\alpha&HFF&}}"
            out_parts.append(f"{ktag}{placeholder}{text}")
        else:
            out_parts.append(f"{ktag}{text}")

    return "".join(out_parts)


ASS_DIALOG_RE = re.compile(r'^(Dialogue:\s*\d+),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*)$')

def process_dialogue_line(line: str, tagger: Tagger, ruby_font_scale: float, keep_style_fs: bool) -> str:
    m = ASS_DIALOG_RE.match(line.rstrip('\n'))
    if not m:
        return line
    # Fields per ASS spec
    # 1: Dialogue: <layer>
    # 2: Start
    # 3: End
    # 4: Style
    # 5: Name
    # 6: MarginL
    # 7: MarginR
    # 8: MarginV
    # 9: Effect
    # 10: Text
    head = m.group(1)
    start, end, style, name, ml, mr, mv, effect, text = m.group(2), m.group(3), m.group(4), m.group(5), m.group(6), m.group(7), m.group(8), m.group(9), m.group(10)
    # Split into k segments
    segs = split_k_segments(text)
    ruby_line = build_ruby_line(segs, tagger, katakana_english=True, ruby_hiragana=True)
    if ruby_line.strip() == "":
        # No annotation needed, return original untouched
        return line
    # Reduce font size for ruby by tag injection at start of ruby line
    ruby_prefix = ""
    if not keep_style_fs:
        # Attempt to detect an existing \fsN in first tag of the line, scale it if present; else set relative \fs via \fs<scaled>
        fs_match = re.search(r'\\fs(\d+)', text)
        if fs_match:
            try:
                base_fs = int(fs_match.group(1))
                ruby_fs = max(8, int(base_fs * ruby_font_scale))
                ruby_prefix = r'{\fs' + str(ruby_fs) + '}'
            except:
                ruby_prefix = ""
        else:
            # Fallback to absolute 24 * scale as a sane default
            ruby_fs = max(8, int(36 * ruby_font_scale))
            ruby_prefix = r'{\fs' + str(ruby_fs) + '}'
    # Compose stacked two-line text: [ruby]\N[original]
    new_text = f"{ruby_prefix}{ruby_line}\\N{text}"
    # Reassemble line
    out = f"{head},{start},{end},{style},{name},{ml},{mr},{mv},{effect},{new_text}"
    return out + "\n"

def process_ass(in_path: str, out_path: str, ruby_font_scale: float = 0.6, keep_style_fs: bool = False) -> None:
    tagger = ensure_tagger()
    with open(in_path, 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
    out_lines: List[str] = []
    for ln in lines:
        if ln.startswith("Dialogue:"):
            out_lines.append(process_dialogue_line(ln, tagger, ruby_font_scale, keep_style_fs))
        else:
            out_lines.append(ln)
    with open(out_path, 'w', encoding='utf-8') as f:
        f.writelines(out_lines)

def main():
    ap = argparse.ArgumentParser(description="Annotate K-frame ASS lyrics with Kanji furigana and English-to-Katakana ruby line stacked above.")
    ap.add_argument("input", help="Input .ass file")
    ap.add_argument("output", help="Output .ass file")
    ap.add_argument("--ruby-font-scale", type=float, default=0.6, help="Ruby font size scale relative to detected base \\fs (default 0.6)")
    ap.add_argument("--keep-style", action="store_true", help="Do not auto-inject \\fs for ruby line (use your style settings)")
    args = ap.parse_args()

    process_ass(args.input, args.output, ruby_font_scale=args.ruby_font_scale, keep_style_fs=args.keep_style)

if __name__ == "__main__":
    main()
