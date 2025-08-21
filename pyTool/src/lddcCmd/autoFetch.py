from collections.abc import Iterable
from functools import reduce
from typing import Literal, overload

from LDDC.common.exceptions import AutoFetchUnknownError, LyricsNotFoundError, NotEnoughInfoError
from LDDC.common.models import APIResultList, Language, LyricInfo, Lyrics, LyricsType, SearchInfo, SearchType, SongInfo, Source
from LDDC.core.algorithm import calculate_artist_score, calculate_title_score, text_difference
from LDDC.core.api.lyrics import get_lyrics, search

@overload
def autoFetch(
    info: SongInfo,
    min_score: float = 60,
    sources: Iterable[Source] = (Source.QM, Source.KG, Source.NE),
    return_search_results: bool = False,
) -> Lyrics: ...


@overload
def autoFetch(
    info: SongInfo,
    min_score: float = 60,
    sources: Iterable[Source] = (Source.QM, Source.KG, Source.NE),
    return_search_results: bool = True,
) -> tuple[Lyrics, APIResultList[SongInfo]]: ...


def autoFetch(
    info: SongInfo,
    min_score: float = 55,
    sources: Iterable[Source] = (Source.QM, Source.KG, Source.NE),
    return_search_results: bool = False,
) -> Lyrics | tuple[Lyrics, APIResultList[SongInfo]]:
    # 关键词准备
    keywords: dict[Literal["artist-title", "title", "file_name"], str] = {}
    if info.title and info.title.strip():
        if info.artist:
            keywords["artist-title"] = info.artist_title()
        keywords["title"] = info.title
    elif info.path:
        keywords["file_name"] = info.path.stem
    else:
        raise NotEnoughInfoError(f"没有足够的信息用于搜索: {info}")

    search_results: dict[SongInfo, APIResultList[SongInfo]] = {}
    songs_score: dict[SongInfo, float] = {}
    lyrics_results: dict[SongInfo, Lyrics] = {}
    errors: list[Exception] = []

    def _search_one(source: Source, keyword: str) -> APIResultList[SongInfo] | None:
        try:
            return search(source, keyword, SearchType.SONG)
        except Exception as e:
            errors.append(e)
            return None

    def _calc_score(base: SongInfo, candidate: SongInfo, keyword: str) -> float:
        if base.duration and candidate.duration and abs(base.duration - candidate.duration) > 4000:
            return -1

        if keyword in (keywords.get("artist-title"), keywords.get("title")):
            title_score = calculate_title_score(base.title or "", candidate.title or "")
            album_score = max(text_difference(base.album.lower(), candidate.album.lower()) * 100, 0) if base.album and candidate.album else None
            artist_score = calculate_artist_score(str(base.artist), str(candidate.artist)) if base.artist and candidate.artist else None

            if artist_score is not None:
                if album_score is not None:
                    score = max(
                        title_score * 0.5 + artist_score * 0.5,
                        title_score * 0.5 + artist_score * 0.35 + album_score * 0.15,
                    )
                else:
                    score = title_score * 0.5 + artist_score * 0.5
            elif album_score:
                score = max(title_score * 0.7 + album_score * 0.3, title_score * 0.8)
            else:
                score = title_score
            if title_score < 30:
                score = max(0, score - 35)
        else:
            score = max(
                text_difference(keyword, base.title or "") * 100,
                text_difference(keyword, f"{base.artist!s} - {base.title}") * 100,
                text_difference(keyword, f"{base.title} - {base.artist!s}") * 100,
            )
        return score

    # 搜索 & 选歌
    keyword = keywords.get("artist-title") or keywords.get("title") or keywords["file_name"]
    for source in sources:
        results = _search_one(source, keyword)
        if results is None or not isinstance(results.info, SearchInfo):
            continue

        scored: list[tuple[float, SongInfo]] = []
        for result in results:
            score = _calc_score(info, result, results.info.keyword)
            if score > min_score:
                scored.append((score, result))

        scored.sort(key=lambda x: x[0], reverse=True)
        if not scored:
            continue

        for score, candidate in scored[:2]:  # 最多尝试两个候选
            try:
                lyrics = get_lyrics(candidate)
                songs_score[candidate] = score
                search_results[candidate] = results
                lyrics_results[candidate] = lyrics
                break
            except Exception as e:
                errors.append(e)
                continue

    # 检查结果
    if not lyrics_results:
        if songs_score:
            sorted_songs_score = [i for i, _ in sorted(songs_score.items(), key=lambda x: x[1], reverse=True) if i.language is not None]
            if sorted_songs_score and sorted_songs_score[0].language == Language.INSTRUMENTAL:
                inst_info = sorted_songs_score[0]
                lyrics_results[inst_info] = Lyrics.get_inst_lyrics(LyricInfo(inst_info.source, inst_info))

        if not lyrics_results:
            if not [e for e in errors if not isinstance(e, LyricsNotFoundError)]:
                raise LyricsNotFoundError("没有找到符合要求的歌曲")
            raise AutoFetchUnknownError("自动获取时发生未知错误", errors)

    # 挑最高分
    highest_score = max(songs_score[song_info] for song_info in lyrics_results)
    lyrics_results = {s: l for s, l in lyrics_results.items() if abs(songs_score[s] - highest_score) <= 15}

    # 优先歌词类型
    have_verbatim = [l for l in lyrics_results.values() if l.types.get("orig") == LyricsType.VERBATIM]
    have_ts = [l for l in lyrics_results.values() if "ts" in l]
    have_roma = [l for l in lyrics_results.values() if "roma" in l]

    def pick_best() -> list[Lyrics]:
        if have_verbatim and have_ts and have_roma:
            return [l for l in lyrics_results.values() if l in have_verbatim and l in have_ts and l in have_roma]
        if have_verbatim and have_ts:
            return [l for l in lyrics_results.values() if l in have_verbatim and l in have_ts]
        if have_ts and have_roma:
            return [l for l in lyrics_results.values() if l in have_ts and l in have_roma]
        if have_ts:
            return have_ts
        if have_verbatim and have_roma:
            return [l for l in lyrics_results.values() if l in have_verbatim and l in have_roma]
        if have_verbatim:
            return have_verbatim
        if have_roma:
            return have_roma
        return list(lyrics_results.values())

    lyrics_list = pick_best()

    for source in sources:
        for lyrics in lyrics_list:
            if lyrics.info.source == source:
                if not return_search_results:
                    return lyrics
                info_key = next(si for si, lr in lyrics_results.items() if lr == lyrics)
                return (
                    lyrics,
                    (search_results[info_key] + reduce(lambda a, b: a + b, [res for si, res in search_results.items() if si != info_key])),
                )

    raise AutoFetchUnknownError("自动获取时发生未知错误", errors)
