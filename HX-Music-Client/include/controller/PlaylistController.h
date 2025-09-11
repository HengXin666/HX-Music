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
            [this, init = false](uint64_t id) mutable {
                GlobalSingleton::get().guiPlaylist = {};
                GlobalSingleton::get().playQueue = {};
                // 发送更新歌单信号
                Q_EMIT SignalBusSingleton::get().playlistChanged(id);
                log::hxLog.info("网络: 请求歌单", id);
                if (id == Playlist::kNonePlaylist) {
                    return;
                }
                PlaylistApi::selectById(id).thenTry([id, &init](container::Try<Playlist> t) mutable {
                    if (!t) [[unlikely]] {
                        GlobalSingleton::get().guiPlaylist = {};
                        log::hxLog.error("请求歌单错误:", t.what());
                        return;
                    }
                    if (!init) {
                        GlobalSingleton::get().nowPlaylist
                            = GlobalSingleton::get().guiPlaylist
                            = t.move();
                        init = true;
                    } else {
                        GlobalSingleton::get().guiPlaylist = t.move();
                    }
                    QMetaObject::invokeMethod(
                        QCoreApplication::instance(),
                        [id] {
                        // 发送更新歌单信号
                        Q_EMIT SignalBusSingleton::get().playlistChanged(id);
                    });
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
            // 加载歌单列表
            Q_EMIT SignalBusSingleton::get().updatePlaylistList(0);
            
            auto idx = GlobalSingleton::get().musicConfig.listIndex;
            if (idx == -1) {
                return;
            }
            // 等待加载好歌单, 然后显示上次的结果
            QTimer::singleShot(500, this, [this, idx] {
                if (auto& songList
                    = GlobalSingleton::get().nowPlaylist.songList; idx >= songList.size()
                ) {
                    log::hxLog.error("网络请求失败! 数据不存在, 导致: 歌单数组越界", idx);
                } else {
                    // 网络加载, 并且显示
                    MusicCommand::switchMusic<false, true>(
                        songList[idx].id
                    );
                    QTimer::singleShot(200, [] {
                        // 设置播放位置, 恢复之前的进度
                        MusicCommand::setMusicPos(
                            GlobalSingleton::get().musicConfig.position
                        );
                    });
                }
            });
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

    /**
     * @brief 创建新歌单
     * @param name 歌单名称
     * @param description 歌单描述
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void makePlaylist(QString name, QString description) {
        PlaylistApi::makePlaylist({
            {},
            name.toStdString(),
            description.toStdString()
        }).thenTry([](container::Try<uint64_t> t) {
            if (!t) [[unlikely]] {
                log::hxLog.error("创建歌单失败:", t.what());
                return;
            }
            Q_EMIT SignalBusSingleton::get().updatePlaylistList(t.move());
        });
    }

    /**
     * @brief 删除歌单
     * @param id 
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void delPlaylist(uint64_t id) {
        PlaylistApi::delPlaylist(id).thenTry([](auto t) {
            if (!t) [[unlikely]] {
                log::hxLog.error("删除歌单失败:", t.what());
                return;
            }
            Q_EMIT SignalBusSingleton::get().updatePlaylistList(0);
        });
    }

    /**
     * @brief 刷新歌单
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void refreshPlaylist() {
        Q_EMIT SignalBusSingleton::get().updatePlaylistList(0);
    }
};

} // namespace HX
