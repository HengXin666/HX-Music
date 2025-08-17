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
#ifndef _HX_CONFIG_MANAGER_H_
#define _HX_CONFIG_MANAGER_H_

#include <config/MusicConfig.hpp>

#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>
#include <HXLibs/utils/FileUtils.hpp>

namespace HX {

/**
 * @brief 配置文件管理器
 */
class ConfigManager {
public:
    void saveConfig(const MusicConfig& config) {
        std::string json;
        reflection::toJson<true>(config, json);
        coroutine::EventLoop loop;
        utils::AsyncFile file{loop};
        file.syncOpen("./hxMusicConfig.json", utils::OpenMode::Write);
        file.syncWrite(json);
        file.syncClose();
    }

    MusicConfig loadConfig() {
        MusicConfig config;
        std::string json;
        try {
            coroutine::EventLoop loop;
            utils::AsyncFile file{loop};
            file.syncOpen("./hxMusicConfig.json", utils::OpenMode::Read);
            json = file.syncReadAll();
            file.syncClose();
        } catch (...) {
            json = R"({
"volume": 100,
"playMode": "ListLoop",
"position": 0,
"musicListId": "localPlaylist",
"listIndex": -1,
"isPlay": false
})";
        }
        reflection::fromJson(config, json);
        return config;
    }
};

} // namespace HX

#endif // !_HX_CONFIG_MANAGER_H_