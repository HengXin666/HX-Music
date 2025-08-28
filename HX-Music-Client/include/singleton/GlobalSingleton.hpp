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

#include <config/ConfigManager.hpp>
#include <utils/MusicPlayer.h>
#include <utils/PlayQueue.hpp>
#include <pojo/Playlist.hpp>

#include <HXLibs/net/client/HttpClient.hpp>

namespace HX {

/**
 * @brief 全局单例
 */
struct GlobalSingleton {
    /**
     * @brief 获取单例
     * @return GlobalSingleton& 
     */
    inline static GlobalSingleton& get() {
        static GlobalSingleton s{};
        return s;
    }

    /// @brief 音频配置 (播放状态)
    MusicConfig musicConfig{};

    /// @brief 当前加载的歌单
    Playlist playlist{};

    /// @brief 音频播放实例
    MusicPlayer music{};

    /// @brief 播放队列
    PlayQueue playQueue{};

    /// @brief 后端 URL
    std::string backendUrl{"http://127.0.0.1"};

    /// @brief Http 客户端
    decltype(net::HttpClient {}) _client{net::HttpClientOptions{}, 4};

    void saveConfig() {
        ConfigManager config;
        config.saveConfig(musicConfig);
    }
private:
    explicit GlobalSingleton() {
        ConfigManager config;
        musicConfig = config.loadConfig();
    }

    GlobalSingleton& operator=(GlobalSingleton&&) = delete;
};

} // namespace HX

