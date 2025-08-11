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

#include <QSettings>

#include <config/MusicConfig.hpp>

namespace HX {

/**
 * @brief 配置文件管理器
 */
class ConfigManager {
public:
    explicit ConfigManager(const QString& fileName = "HX-Music.ini")
        : _settings("HX", fileName)
    {}

    void saveConfig(const MusicConfig& config) {
        _settings.setValue("Music/Volume", config.volume);
        _settings.setValue("Music/PlayMode", static_cast<int>(config.playMode));
        _settings.setValue("Music/Position", config.position);
        _settings.setValue("Music/ListIndex", config.listIndex);
        _settings.setValue("Music/MusicListId", QString::fromStdString(config.musicListId));
    }

    MusicConfig loadConfig() {
        MusicConfig config;
        config.volume = _settings.value("Music/Volume", 0.75).toFloat();
        config.playMode = static_cast<PlayMode>(_settings.value("Music/PlayMode", static_cast<int>(PlayMode::ListLoop)).toInt());
        config.position = _settings.value("Music/Position", 0).toLongLong();
        config.musicListId = _settings.value("Music/MusicListId", "localPlaylist").toString().toStdString();
        config.listIndex = _settings.value("Music/ListIndex", -1).toInt();
        return config;
    }

private:
    QSettings _settings; // qt保证其析构会调用`sync()`
};

} // namespace HX

#endif // !_HX_CONFIG_MANAGER_H_