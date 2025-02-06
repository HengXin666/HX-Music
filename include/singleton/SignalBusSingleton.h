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
#ifndef _HX_SIGNAL_BUS_SINGLETON_H_
#define _HX_SIGNAL_BUS_SINGLETON_H_

#include <QObject>
#include <QMutex>

/**
 * @brief 信号总线单例
 */
class SignalBusSingleton : public QObject {
    Q_OBJECT

    explicit SignalBusSingleton() noexcept {}
    SignalBusSingleton& operator=(SignalBusSingleton&&) = delete;

public:
    /**
     * @brief `线程安全`获取信号总线单例
     * @return SignalBusSingleton& 
     */
    inline static SignalBusSingleton& get() {
        static QMutex mutex{};
        QMutexLocker _{&mutex};
        static SignalBusSingleton singleton{};
        return singleton;
    }

Q_SIGNALS:
    // 加载新歌
    void NewSongLoaded();
};

#endif // !_HX_SIGNAL_BUS_SINGLETON_H_