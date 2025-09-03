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

#include <QTimer>

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/ImagePool.h>
#include <cmd/MusicCommand.hpp>
#include <api/PlaylistApi.hpp>

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
            [this](uint64_t id) {
                GlobalSingleton::get().playlist = {};
                if (id == Playlist::kLocalPlaylist) {
                    // 加载本地
                    try {
                        coroutine::EventLoop loop;
                        utils::AsyncFile file{loop};
                        std::string json;
                        file.syncOpen("./localPlaylist.json", platform::OpenMode::Read);
                        json = file.syncReadAll();
                        file.syncClose();
                        reflection::fromJson(GlobalSingleton::get().playlist, json);
                    } catch (...) {
                        GlobalSingleton::get().playlist = {
                            Playlist::kLocalPlaylist,
                            "本地歌单",
                            {}
                        };
                    }
                    // 发送更新歌单信号
                    Q_EMIT SignalBusSingleton::get().playlistChanged(id);
                } else {
                    // @todo 网络
                    log::hxLog.info("网络: 请求歌单", id);
                    PlaylistApi::selectById(id).thenTry([=](container::Try<Playlist> t){
                        if (!t) [[unlikely]] {
                            GlobalSingleton::get().playlist = {};
                            log::hxLog.error("请求歌单错误:", t.what());
                        } else {
                            GlobalSingleton::get().playlist = t.move();
                        }
                        // 发送更新歌单信号
                        Q_EMIT SignalBusSingleton::get().playlistChanged(id);
                    });
                }
            });
        
        // 保存歌单
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::savePlaylistSignal,
            this,
            [this]() {
            auto& playlist = GlobalSingleton::get().playlist;
            if (playlist.id == Playlist::kLocalPlaylist) {
                // 保存本地
                coroutine::EventLoop loop;
                utils::AsyncFile file{loop};
                file.syncOpen("./localPlaylist.json");
                std::string json;
                reflection::toJson<true>(playlist, json);
                file.syncWrite(json);
                file.syncClose();
            } else {
                // @todo 网络
                log::hxLog.warning("网络版本没有实现!, ErrId:", playlist.id);
            }
        });

        // === init ===
        // 加载配置歌单
        loadPlaylistById(GlobalSingleton::get().musicConfig.playlistId);
        // 显示上次加载的歌曲
        QTimer::singleShot(0, this, [this] {
            auto idx = GlobalSingleton::get().musicConfig.listIndex;
            if (idx == -1) {
                return;
            }
            if (!GlobalSingleton::get().musicConfig.playlistId) {
                auto path = QString::fromStdString(
                    GlobalSingleton::get().playlist.songList[idx].path);
                MusicInfo musicInfo{QFileInfo{path}};
                if (auto img = musicInfo.getAlbumArtAdvanced()) {
                    ImagePoll::get()->add(path, img->toImage());
                }
                // 本地歌曲
                MusicCommand::switchMusic<false, true>(path);
            } else {
                // 网络加载
                MusicCommand::switchMusic<false, true>(
                    QString{"%1"}.arg(GlobalSingleton::get().playlist.songList[idx].id)
                );
            }
        });
    }

    /**
     * @brief 加载歌单
     * @param id 歌单id
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void loadPlaylistById(uint64_t id) {
        Q_EMIT SignalBusSingleton::get().loadPlaylistSignal(id);
    }
};

} // namespace HX
