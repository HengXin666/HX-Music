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
#ifndef _HX_ASS_LYRIC_WIDGET_H_
#define _HX_ASS_LYRIC_WIDGET_H_

#include <QWidget>

#include <utils/AssParse.hpp>
#include <utils/MusicInfo.hpp>

/**
 * @brief Ass歌词渲染控件
 */
class AssLyricWidget : public QWidget {
    Q_OBJECT
public:
    explicit AssLyricWidget(QWidget* parent = nullptr);

    /**
     * @brief 添加字幕偏移量
     * @param off 偏移量 (单位: 毫秒(ms))
     */
    void addOffset(long long off) {
        _offset += off;
    }

    /**
     * @brief 重置字幕偏移量
     */
    void resetOffset() {
        _offset = 0;
    }

    /**
     * @brief 设置是否处于移动状态
     * @param isMove 
     * \true 是, 可以移动: 绘制辅助背景
     * \false 否, 不可移动: 背景为透明的
     */
    void setMoveFlag(bool isMove) {
        _isMove = isMove;
    }

protected:
    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent* event) override;
private:
    /**
     * @brief 查找该歌曲的歌词文件, 并且加载
     * @param it 歌曲迭代器
     */
    void findLyricFile(HX::MusicInfo const& info);

    /**
     * @brief 渲染字幕
     * @param nowTime 当前时间 (单位: 毫秒(ms))
     */
    void updateLyric(qint64 nowTime);
    
    QImage _img;
    HX::AssParse _assParse;
    long long _offset = 0;  // 字幕偏移量
    bool _isMove{false};    // 是否处于移动
};

#endif // !_HX_ASS_LYRIC_WIDGET_H_