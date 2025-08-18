#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ass_furi_from_kf.py
Convert Dialogue lines that contain per-character {\\kfN} tags into:
 - original Dialogue (unchanged)
 - a katakana->hiragana kf-preserved furigana Dialogue (if leading katakana run exists)
 - a bracketed-reading (parentheses) kf-preserved furigana Dialogue (if exists)

Usage:
  python3 ass_furi_from_kf.py -i input.ass -o output.ass
  cat input.ass | python3 ass_furi_from_kf.py > output.ass
"""

import re
import sys
import argparse
from typing import List, Tuple, Optional

# regex to capture Dialogue fields up to Text (Text may contain commas)
DIALOGUE_RE = re.compile(
    r'^(Dialogue:\s*)([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),(.*)$',
    flags=re.UNICODE
)

KF_CHAR_RE = re.compile(r'(\{\\kf(\d+)\})(.)', flags=re.DOTALL)

# Katakana Unicode range for basic mapping
KATAKANA_START = 0x30A1
KATAKANA_END = 0x30FA

def is_katakana(ch: str) -> bool:
    if not ch:
        return False
    cp = ord(ch)
    return KATAKANA_START <= cp <= KATAKANA_END or cp == 0x30FC  # include prolonged sound mark

def kata_to_hira(ch: str) -> str:
    cp = ord(ch)
    # Prolonged sound mark U+30FC keeps as is
    if cp == 0x30FC:
        return ch
    if KATAKANA_START <= cp <= KATAKANA_END:
        return chr(cp - 0x60)  # common katakana->hiragana offset
    return ch

def extract_kf_tokens(text: str) -> List[Tuple[str, str]]:
    """
    Return list of tuples (kf_tag, char) in the order they appear.
    kf_tag includes the full "{\\kfNN}" string.
    """
    tokens = KF_CHAR_RE.findall(text)
    # each match is (full_tag, number, char)
    return [(full, ch) for (full, _, ch) in tokens]

def build_text_from_kf_tokens(tokens: List[Tuple[str, str]]) -> str:
    return ''.join(kf + ch for kf, ch in tokens)

def find_parentheses_token_range(tokens: List[Tuple[str, str]]) -> Optional[Tuple[int, int]]:
    """
    Find first '(' token and its matching ')' position in token list.
    Return (start_idx, end_idx) for inner content (excluding paren tokens).
    If not found, return None.
    """
    # locate token indices where char is '(' or ')'
    start = None
    for i, (_, ch) in enumerate(tokens):
        if ch == '(' or ch == '（':
            start = i
            break
    if start is None:
        return None
    # find closing
    for j in range(start + 1, len(tokens)):
        if tokens[j][1] == ')' or tokens[j][1] == '）':
            return (start + 1, j)  # inner range excludes parentheses
    return None

def find_leading_katakana_run(tokens: List[Tuple[str, str]]) -> Optional[Tuple[int, int]]:
    """
    Find contiguous run starting from token 0 that are katakana (or small punctuation allowed).
    Return (start_idx, end_idx_exclusive) or None if no leading katakana tokens.
    """
    if not tokens:
        return None
    i = 0
    # skip initial tokens that are whitespace or control? But tokens come from {\kf} matches only.
    # Accept run while token char is katakana or prolonged mark or small kana (they fall into range).
    if not is_katakana(tokens[0][1]):
        return None
    while i < len(tokens) and is_katakana(tokens[i][1]):
        i += 1
    return (0, i)

def make_furi_line_template(prefix_override: str = r'{\an5\fs10}') -> str:
    # Style furi-fx by default, effect empty
    # Fields: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
    # We'll fill Layer/Start/End from original and keep Name/Margins empty
    # The function that builds full line will prefix this string to the text.
    return prefix_override

def process_dialogue_line(line: str) -> List[str]:
    """
    Returns a list of output dialogue lines for this input dialogue line.
    Usually returns [original_line] or [original_line, furi_kata_line, furi_paren_line...]
    """
    m = DIALOGUE_RE.match(line)
    if not m:
        return [line]
    prefix = m.group(1)  # "Dialogue:   "
    layer = m.group(2)
    start = m.group(3)
    end = m.group(4)
    style = m.group(5)
    name = m.group(6)
    marginL = m.group(7)
    marginR = m.group(8)
    marginV = m.group(9)
    text = m.group(10)

    outputs = [line.rstrip('\n')]

    # find all kf tokens
    tokens = extract_kf_tokens(text)
    if not tokens:
        # no per-char kf tokens, nothing to do
        return outputs

    # leading katakana run
    kata_range = find_leading_katakana_run(tokens)
    if kata_range:
        s, e = kata_range
        kata_tokens = tokens[s:e]
        # convert each token's char to hiragana, keep same kf tag
        converted = ''.join(kf + kata_to_hira(ch) for (kf, ch) in kata_tokens)
        # prefix override and build full Text
        override = make_furi_line_template()
        newText = override + converted
        # Build new Dialogue line with Style furi-fx, Name blank, Effect blank
        newLine = f"{prefix}{layer},{start},{end},furi-fx,,{marginL},{marginR},{marginV},,{newText}"
        outputs.append(newLine)

    # parentheses reading
    par_range = find_parentheses_token_range(tokens)
    if par_range:
        s, e = par_range
        inner_tokens = tokens[s:e]
        if inner_tokens:
            # build text from inner tokens, preserve kf tags
            converted_inner = ''.join(kf + ch for (kf, ch) in inner_tokens)
            override = make_furi_line_template()
            newText = override + converted_inner
            newLine = f"{prefix}{layer},{start},{end},furi-fx,,{marginL},{marginR},{marginV},,{newText}"
            outputs.append(newLine)

    return outputs

def process_ass(text: str) -> str:
    lines = text.splitlines()
    out_lines: List[str] = []
    in_events = False
    for line in lines:
        out_lines.append(line) if not line.startswith('Dialogue:') else None
        if line.strip().lower().startswith('[events]'):
            in_events = True
            # keep the [Events] line
            out_lines[-1] = line
            continue
        if not in_events:
            # outside events block, just copy
            continue
        # inside events, process Dialogue lines
        if line.startswith('Dialogue:'):
            new_lines = process_dialogue_line(line)
            # append processed lines
            for nl in new_lines:
                out_lines.append(nl)
        else:
            # non-Dialogue lines inside events (e.g. Format: ...) keep as-is
            if not line.startswith('Dialogue:'):
                out_lines.append(line)

    # If we appended some lines twice due to initial append logic, clean duplicates preserving order
    # Simpler: rebuild by iterating original and processing
    # For safety, produce fresh output by re-processing
    final_out: List[str] = []
    in_events = False
    for line in lines:
        final_out.append(line) if not line.startswith('Dialogue:') else None
        if line.strip().lower().startswith('[events]'):
            in_events = True
            final_out[-1] = line
            continue
        if not in_events:
            continue
        if line.startswith('Dialogue:'):
            for nl in process_dialogue_line(line):
                final_out.append(nl)
        else:
            final_out.append(line)

    return '\n'.join(final_out) + '\n'

def main():
    parser = argparse.ArgumentParser(description='Convert ASS Dialogue with per-char {\\kfN} into lines with furigana kf preserved.')
    parser.add_argument('-i', '--input', help='input ASS file (default stdin)', default=None)
    parser.add_argument('-o', '--output', help='output ASS file (default stdout)', default=None)
    args = parser.parse_args()

    if args.input:
        with open(args.input, 'r', encoding='utf-8') as f:
            src = f.read()
    else:
        src = sys.stdin.read()

    out = process_ass(src)

    if args.output:
        with open(args.output, 'w', encoding='utf-8') as f:
            f.write(out)
    else:
        sys.stdout.write(out)

if __name__ == '__main__':
    main()
