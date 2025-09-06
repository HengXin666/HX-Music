import re
from pathlib import Path
from typing import Any, Callable, List

import ass

from src.mark.AssMark import AssMark
from src.lddcCmd.localMatcAss import localMatcAss
from src.utils.callLua import callApplyKaraokeTemplateLua, callSetKaraokeStyleLua

def _addSy01(styles: List[ass.document.Style]) -> List[ass.document.Style]:
    # 添加样式
    def makeStyle(name: str, font: str, size: int,
                  primary: str, secondary: str,
                  outline: str, back: str,
                  bold: int, italic: int,
                  borderStyle: int, outlineW: int,
                  shadow: int, align: int,
                  ml: int, mr: int, mv: int, encoding: int) -> ass.document.Style:
        s = ass.document.Style()
        s.name = name
        s.fontname = font
        s.fontsize = size
        s.primary_color = primary
        s.secondary_color = secondary
        s.outline_color = outline
        s.back_color = back
        s.bold = bold
        s.italic = italic
        s.border_style = borderStyle
        s.outline = outlineW
        s.shadow = shadow
        s.alignment = align
        s.margin_l = ml
        s.margin_r = mr
        s.margin_v = mv
        s.encoding = encoding
        return s

    styles.extend([
        makeStyle("noK-furigana", "TakaoPGothic", 40,
                  "&H0039FFFF", "&H00FFFFFF",
                  "&H00000000", "&H80000000",
                  0, 0, 1, 2, 0, 2, 0, 0, 440, 1),
        makeStyle("K2-furigana", "TakaoPGothic", 40,
                  "&H00FFFFFF", "&H00FFFFFF",
                  "&H00000000", "&H80000000",
                  0, 0, 1, 2, 0, 3, 30, 120, 40, 1),
        makeStyle("K1-furigana", "TakaoPGothic", 40,
                  "&H00FFFFFF", "&H00FFFFFF",
                  "&H00000000", "&H80000000",
                  0, 0, 1, 2, 0, 1, 120, 30, 240, 1),
        makeStyle("K1", "TakaoPGothic", 80,
                  "&H00FFFFFF", "&H00FFFFFF",
                  "&H00000000", "&H80000000",
                  0, 0, 1, 4, 0, 1, 120, 30, 240, 1),
        makeStyle("K2", "Arial", 80,
                  "&H00FFFFFF", "&H00FFFFFF",
                  "&H00000000", "&H80000000",
                  0, 0, 1, 4, 0, 3, 30, 120, 40, 1),
        makeStyle("noK", "TakaoPGothic", 80,
                  "&H0039FFFF", "&H00FFFFFF",
                  "&H00000000", "&H80000000",
                  0, 0, 1, 4, 0, 2, 0, 0, 440, 1),
    ])

    return styles

def addParsedComments(list: List[Any], lines: List[str]) -> List[Any]:
    """
    lines: List[str]，每条是 Comment: ... 的 ASS 行
    """
    pattern = re.compile(
        r'^Comment:\s*(\d+),'           # Layer
        r'([\d:.\s]+),'                  # Start
        r'([\d:.\s]+),'                  # End
        r'([^,]*),'                       # Style
        r'([^,]*),'                       # Name
        r'(\d+),'                         # MarginL
        r'(\d+),'                         # MarginR
        r'(\d+),'                         # MarginV
        r'([^,]*),'                        # Effect
        r'(.*)$'                           # Text
    )

    for line in lines:
        m = pattern.match(line)
        if not m:
            raise ValueError(f"Line format error: {line}")

        evt = ass.document.Comment()
        evt.layer = int(m.group(1))
        evt.start = m.group(2).strip()
        evt.end = m.group(3).strip()
        evt.style = m.group(4).strip()
        evt.name = m.group(5).strip()
        evt.margin_l = int(m.group(6))
        evt.margin_r = int(m.group(7))
        evt.margin_v = int(m.group(8))
        evt.effect = m.group(9).strip()
        evt.text = m.group(10).strip()

        list.append(evt)
    return list


def _addTx01(list: List[Any]) -> List[Any]:
    # karaok函数
    lines = [
        r"Comment: 0,0:00:00.00,0:00:00.00,K1,,0,0,0,code once,line_strech = 0.7; colors = {{'&HFFFFFF&','&H080808&','&H0000B4&','&HFFFFFF&'},}; prevline = {}; line_count = 0",
        r'Comment: 0,0:00:00.00,0:00:00.00,K1,,0,0,0,code line all,line_count = line_count + 1; is_second_line = (line_count % 2 == 0); base_y = is_second_line and 650 or 439; x = 100; y = base_y',
        r"Comment: 0,0:00:00.00,0:00:00.00,K1,,0,0,0,code syl furi all,if syl.inline_fx~='' then style_index = string.byte(syl.inline_fx)-64 else style_index = 1 end",
        r'Comment: 0,0:00:00.00,0:00:00.00,K1,,0,0,0,code line all,if prevline[line.style]  then pline = prevline[line.style] end',
        r'Comment: 0,0:00:00.00,0:00:00.00,K1,,0,0,0,code line all,interdur = {} if pline then interdur[line.styleref] = line.start_time - pline.end_time   end',
        r'Comment: 0,0:00:00.00,0:00:00.00,K1,set fxgroup and firstline,0,0,0,code line all,fxgroup.base =  1',
        r"Comment: 0,0:00:00.00,0:00:00.00,K1,set fxgroup and firstline,0,0,0,code syl furi all,fxgroup.firstline_overlay = syl.duration>10 fxgroup.has_prevline_overlay = syl.duration>10 fxgroup.overlay = line.styleref.name~='noK' and syl.duration>0",
        r'Comment: 0,0:00:00.00,0:00:00.00,K1,,0,0,0,code line all,prevline[line.style] = line',
        r'Comment: 0,0:00:05.00,0:00:05.00,K1,base,0,0,0,template syl furi fxgroup base all,!retime("line")!{\an5\pos(!math.floor(x+syl.width/2+syl.left)!,!syl.isfuri and math.floor(y-syl.height*3/2) or y !)}{\1c!colors[style_index][1]!\3c!colors[style_index][2]!}',
        r'Comment: 0,0:00:05.00,0:00:05.00,K1,overlay,0,0,0,template syl furi fxgroup overlay all,!relayer(2)!!retime("line")!{\an5\pos(!math.floor(x+syl.width/2+syl.left)!,!syl.isfuri and math.floor(y-syl.height*3/2) or y!)\shad0\clip(!x+syl.left-line.styleref.outline-1!,0,!x+syl.left-line.styleref.outline-1!,1080)\t($sstart,$send,\clip(!x+syl.left-line.styleref.outline-1!,0,!x+syl.right+line.styleref.outline+1!,1080))\bord!line.styleref.outline+1!}{\1c!colors[style_index][3]!\3c!colors[style_index][4]!}',
    ]
    return addParsedComments(list, lines)


class KaRaOKAss:
    """卡拉ok Ass 相关操作类"""
    def __init__(self) -> None:
        self._assMark = AssMark()
    
    def _lddcAssToKaraokeAss(self, inFile: str, outFile: str):
        """进行日语注音

        Args:
            inFile (str): 输入路径
            outFile (str): 输出路径
        """
        with open(inFile, "r", encoding="utf_8_sig") as f:
            doc = ass.parse(f)

        doc.styles = _addSy01(doc.styles)
        
        newEvents = _addTx01([])
        flag: bool = True
        for evt in doc.events:
            # 只处理 Dialogue, 且包含 \k 或 \kf 的
            if evt.TYPE == "Dialogue" and ("\\k" in evt.text or "\\kf" in evt.text):
                evt.TYPE = "Comment"
                evt.effect = "karaoke"
                evt.text = self._assMark.mark(evt.text)
                flag = not flag
            newEvents.append(evt)

        doc.info['PlayResX'] = 1920
        doc.info['PlayResY'] = 1080
        doc.events = newEvents
        with open(outFile, "w", encoding="utf_8_sig") as f:
            # 手动加注释行
            f.write("; HX-Music - PyTool: https://github.com/HengXin666/HX-Music\n")
            f.write("; By Heng_Xin\n")
            doc.dump_file(f)


    def findLyricsFromNet(self, absolutePath: str, outputPath: str, minScore: int = 60):
        """从本地文件在网络上匹配歌词并保存到指定路径

        Args:
            absolutePath (str): 本地歌曲的绝对路径
            outputPath (str): 输出路径 (绝对路径)
        """
        localMatcAss(Path(absolutePath), Path(outputPath), minScore)
    
    def doJapanesePhonetics(self, absolutePath: str):
        """对歌词进行日语注音

        Args:
            absolutePath (str): 歌词的绝对路径
        """
        self._lddcAssToKaraokeAss(absolutePath, absolutePath)

    def toTwoLineKaraokeStyle(self, absolutePath: str):
        """转变为双行卡拉ok样式

        Args:
            absolutePath (str): 歌词的绝对路径
        """
        callSetKaraokeStyleLua(Path(absolutePath), Path(absolutePath), "orig")

    def callApplyKaraokeTemplateLua(self, absolutePath: str):
        """应用卡拉ok模板

        Args:
            absolutePath (str): 歌词的绝对路径
        """
        callApplyKaraokeTemplateLua(Path(absolutePath), Path(absolutePath))


if __name__ == "__main__":
    from src.mark.jpMark import JpMark
    # {\\k10}{\\kf12}い{\\kf14}つ{\\kf14}も{\\kf14}こ{\\kf14}の{\\kf12}場{\\kf16}所{\\kf8}で
    # {\\kf8} (
    # {\\kf14}一{\\kf13}直{\\kf15}在{\\kf16}这{\\kf14}个
    # {\\kf16}地{\\kf17}方
    # {\\kf12}) - {\\kf33}彩{\\kf7}音{\\kf7} ({\\kf14}あ{\\kf17}や{\\kf16}ね{\\kf0})
    _ass = AssMark()
    s = _ass.mark('{\\kf14}日{\\kf13}々')
    print(s)
    # 示例: 简单回调, 把所有 \kf 改为 \k
    # lddcAssToKaraokeAss("test.ass", "output.ass")
    pass