#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HX-Music.
 *
 * HX-Music is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HX-Music is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HX-Music.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstdint>
#include <optional>
#include <string>

namespace HX {

enum class WsLyricsMsgEnum {
    CrawlLyrics,            // 爬取歌词
    CrawlLyricsGetList,     // 爬取歌词并返回结果列表
    JpTranscription,        // 日语注音
    TwoLineKaraokeStyle,    // 双行卡拉ok化
    CallKaraokeTemplateLua, // 应用卡拉ok模板
};

/// @note 客户端使用 WsLyricsMsgVO<> 请求, 服务端返回 WsLyricsMsgVO<...> 给客户端
template <typename T = std::string>
struct WsLyricsMsgVO {
    uint64_t musicId;       // 歌曲id
    WsLyricsMsgEnum type;   // 类型
    std::optional<T> data;  // 可能的数据
};

} // namespace HX