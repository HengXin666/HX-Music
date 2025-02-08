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
#ifndef _HX_GLOBAL_SINGLETON_H_
#define _HX_GLOBAL_SINGLETON_H_

#include <config/ConfigManager.hpp>
#include <views/InterfaceManagementProxy.h>
#include <utils/MusicPlayer.h>
#include <utils/PlayQueue.hpp>

inline int __test__ = []{
    HX::PlayQueue pq;
    pq.insert(nullptr, HX::PlayQueue::Tp {{}, {}});
    pq.insert(nullptr, HX::PlayQueue::Tp {{}, {}});
    pq.next();
    pq.next();
    // pq.next(); err
    return 0;
}();

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

    /// @brief 界面管理代理类
    InterfaceManagementProxy imp{};

    /// @brief 音频播放实例
    HX::MusicPlayer music{};

    /// @brief 音频配置
    MusicConfig musicConfig{};

    /// @brief 播放队列
    HX::PlayQueue playQueue{};
private:
    explicit GlobalSingleton() {
        ConfigManager config;
        musicConfig = config.loadConfig();
    }
    
    ~GlobalSingleton() noexcept {
        ConfigManager config;
        config.saveConfig(musicConfig);
    }

    GlobalSingleton& operator=(GlobalSingleton&&) = delete;
};

#endif // !_HX_GLOBAL_SINGLETON_H_
