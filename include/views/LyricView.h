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
#ifndef _HX_LYRIC_VIEW_H_
#define _HX_LYRIC_VIEW_H_

#include <QWidget>

#include <widget/AssLyricWidget.h>
#include <widget/SvgIconPushButton.h>

/**
 * @brief 歌词界面
 */
class LyricView : public QWidget {
    Q_OBJECT
public:
    explicit LyricView(QWidget* parent = nullptr);

    /**
     * @brief 显示歌词操作栏
     */
    void showSettingView();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
private:
    AssLyricWidget* _lyricWidget = new AssLyricWidget(this);
    QPoint _relativePos;    // 相对位置
    SvgIconPushButton* _btnUnLock;
    bool _isMove{false};    // 正在移动字幕位置
};

#endif // !_HX_LYRIC_VIEW_H_