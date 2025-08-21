import os
from pathlib import Path

from LDDC.common.models._info import SongInfo

def saveAss(song: SongInfo, assText: str, savePath: str = "."):
    """
    保存 Ass 到文件
    """
    filename = f"{song.title} - {song.artist}.ass"
    filename = "".join(c for c in filename if c not in r'\/:*?"<>|')
    filepath = os.path.join(savePath, filename)
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(assText)

def saveAssByPath(assText: str, savePath: Path):
    """
    保存 Ass 到文件, 可指定保存路径
    """
    with open(savePath.absolute(), "w", encoding="utf-8") as f:
        f.write(assText)
