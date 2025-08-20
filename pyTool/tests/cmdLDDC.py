import os

import include

from LDDC.common.models._enums import LyricsFormat
from LDDC.common.models import Source, SearchType
from LDDC.core.api.lyrics import search, get_lyrics
from LDDC.core.converter import convert2

def saveAss(song, ass_text, save_path="."):
    filename = f"{song.title} - {song.artist}.ass"
    filename = "".join(c for c in filename if c not in r'\/:*?"<>|')
    filepath = os.path.join(save_path, filename)
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(ass_text)
    print(f"已保存: {filepath}")

def convert_to_lrc(lyric):
    lrc_text = ""
    for line in lyric['orig']:
        words = line.words
        if words:
            start_time = line.start
            text = "".join([word.text for word in words])
            minutes = start_time // 60000
            seconds = (start_time % 60000) // 1000
            milliseconds = start_time % 1000
            lrc_line = f"[{minutes:02d}:{seconds:02d}.{milliseconds:03d}]{text}\n"
            lrc_text += lrc_line
    return lrc_text

def download_lyrics(song_name, save_path="."):
    # 指定搜索源 QQ music
    result = search(Source.QM, song_name, SearchType.SONG)
    for song in result:
        print(f"{song.title} - {song.artist}")
        try:
            lyric = get_lyrics(song)
            # print(f"Lyric object: {lyric}")  # 打印 lyric 对象的内容

            # 转为 Ass
            assText = convert2(lyric, ["orig"], LyricsFormat.ASS)
            saveAss(song, assText, save_path)
        except Exception as e:
            print(f"获取歌词失败: {e}")
        break  # 只保存第一首，去掉break可批量保存

if __name__ == "__main__":
    song_name = "夏洛特op"
    save_path = "./"  # 你可以修改为你想要保存的路径
    os.makedirs(save_path, exist_ok=True)  # 确保保存路径存在
    download_lyrics(song_name, save_path)