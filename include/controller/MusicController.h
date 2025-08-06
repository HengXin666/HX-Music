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
#ifndef _HX_MUSIC_CONTROLLER_H_
#define _HX_MUSIC_CONTROLLER_H_

#include <QObject>

namespace HX {

/**
 * @brief 音乐控制器
 *        用于控制音乐播放状态的QML交互对象
 * 可以控制:
 *  - 播放/暂停
 *  - 上一首/下一首
 *  - 设置播放模式: 单曲循环/顺序/随机
 *  - 拖动条跳转到某位置
 */
class MusicController : public QObject {
    Q_OBJECT
public:
    
};

} // namespace HX

#endif // !_HX_MUSIC_CONTROLLER_H_