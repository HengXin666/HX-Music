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
#ifndef _HX_PLAYLIST_CONTROLLER_H_
#define _HX_PLAYLIST_CONTROLLER_H_

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>

#include <HXLibs/utils/FileUtils.hpp>
#include <HXLibs/log/Log.hpp>
#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>

namespace HX {

class PlaylistController : public QObject {
    Q_OBJECT
public:
    PlaylistController(QObject* p = nullptr)
        : QObject{p}
    {
        // 订阅歌单加载信号
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::loadPlaylistSignal,
            this,
            [this](std::string const& id) {
                if (id == "localPlaylist") {
                    // 加载本地
                    coroutine::EventLoop loop;
                    utils::AsyncFile file{loop};
                    std::string json;
                    try {
                        file.syncOpen("./localPlaylist.json", platform::OpenMode::Read);
                        json = file.syncReadAll();
                        file.syncClose();
                    } catch (...) {
                        json = R"({
    "playlistId": "localPlaylist",
    "playlistDescription": "本地歌单",
    "songList": []
})";
                    }
                    MusicList musicList;
                    reflection::fromJson(musicList, json);
                    GlobalSingleton::get().musicList = std::move(musicList);
                } else {
                    // @todo 网络
                    log::hxLog.warning("网络版本没有实现!, ErrId:", id);
                }

                // 发生更新歌单信号
                Q_EMIT SignalBusSingleton::get().playlistChanged();
            });
        
        // 保存歌单
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::savePlaylistSignal,
            this,
            [this]() {
            auto& musicList = GlobalSingleton::get().musicList;
            if (musicList.playlistId == "localPlaylist") {
                // 保存本地
                coroutine::EventLoop loop;
                utils::AsyncFile file{loop};
                file.syncOpen("./localPlaylist.json");
                std::string json;
                reflection::toJson<true>(musicList, json);
                file.syncWrite(json);
                file.syncClose();
            } else {
                // @todo 网络
                log::hxLog.warning("网络版本没有实现!, ErrId:", musicList.playlistId);
            }
        });

        // === init ===
        // 加载配置歌单
        Q_EMIT SignalBusSingleton::get().loadPlaylistSignal(
            GlobalSingleton::get().musicConfig.musicListId
        );
    }
};

} // namespace HX

#endif // !_HX_PLAYLIST_CONTROLLER_H_