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
                GlobalSingleton::get().guiPlaylist = {};
                log::hxLog.info("网络: 请求歌单", id);
                if (id == Playlist::kNonePlaylist) {
                    return;
                }
                PlaylistApi::selectById(id).thenTry([=](container::Try<Playlist> t){
                    if (!t) [[unlikely]] {
                        GlobalSingleton::get().guiPlaylist = {};
                        log::hxLog.error("请求歌单错误:", t.what());
                    } else {
                        GlobalSingleton::get().guiPlaylist = t.move();
                    }
                    if (id == GlobalSingleton::get().musicConfig.playlistId) {
                        GlobalSingleton::get().nowPlaylist = GlobalSingleton::get().guiPlaylist;
                    }
                    // 发送更新歌单信号
                    Q_EMIT SignalBusSingleton::get().playlistChanged(id);
                });
            });
        
        // 保存歌单
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::savePlaylistSignal,
            this,
            [this]() {
            auto& playlist = GlobalSingleton::get().guiPlaylist;
            // @todo 网络
            log::hxLog.warning("网络版本没有实现!, ErrId:", playlist.id);
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
            // 网络加载
            MusicCommand::switchMusic<false, true>(
                QString{"%1"}.arg(GlobalSingleton::get().nowPlaylist.songList[idx].id)
            );
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
