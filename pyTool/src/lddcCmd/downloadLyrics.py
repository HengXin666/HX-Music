from LDDC.common.models._enums import SearchType, Source, LyricsFormat
from LDDC.core.converter import convert2
from LDDC.core.api.lyrics import search, get_lyrics

from .saveAss import saveAss

def downloadLyrics(song_name, save_path="."):
    """通过搜索下载歌词

    Args:
        song_name (_type_): 歌曲名称
        save_path (str, optional): 输出路径. Defaults to ".".
    """
    # 指定搜索源 QQ music
    songInfoList = search(Source.QM, song_name, SearchType.SONG)
    for song in songInfoList:
        print(f"{song.title} - {song.artist}")
        try:
            lyric = get_lyrics(song)
            # 转为 Ass
            assText = convert2(lyric, ["orig"], LyricsFormat.ASS)
            saveAss(song, assText, save_path)
        except Exception as e:
            print(f"获取歌词失败: {e}")
        break  # 只保存第一首，去掉break可批量保存