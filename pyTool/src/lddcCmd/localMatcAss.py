from pathlib import Path

from LDDC.common.models._enums import LyricsFormat
from LDDC.common.models import Source
from LDDC.core.converter import convert2
from LDDC.core.song_info import audio_formats
from LDDC.common.models._info import SongInfo
from LDDC.core.song_info import get_audio_file_infos
from .autoFetch import autoFetch
from .saveAss import saveAssByPath

def _parseSingleSong(path: Path) -> SongInfo:
    """
    解析单个音频文件, 返回一个 SongInfo.
    仅支持单曲文件, 不支持 CUE/多曲封装.
    """
    if not path.exists() or not path.is_file():
        raise FileNotFoundError(f"无效的文件路径: {path}")

    if path.suffix.lower().removeprefix(".") not in audio_formats:
        raise ValueError(f"不支持的文件类型: {path.suffix}")

    try:
        infos = get_audio_file_infos(path)
    except Exception as e:
        raise RuntimeError(f"解析音频文件失败 {path}: {e}") from e

    if len(infos) != 1:
        raise ValueError(f"期望单曲文件, 但得到 {len(infos)} 个曲目: {path}")

    return infos[0]

# 本地匹配
def localMatcAss(localMusicPath: Path, outputAssPath: Path, minScore: float = 60):
    """匹配本地歌曲

    Args:
        localMusicPath (Path): 本地歌曲路径
        outputAssPath (Path): 输出路径
        minScore (float, optional): 最小相似度. Defaults to 60.
    """
    # 搜索
    info = _parseSingleSong(localMusicPath)
    lyric = autoFetch(info=info,
                       min_score=minScore,
                       sources=(Source.QM, Source.KG, Source.NE),
                       return_search_results=False)
    # 转为 Ass
    assText = convert2(lyric, ["orig"], LyricsFormat.ASS)

    # 保存
    saveAssByPath(assText, outputAssPath)