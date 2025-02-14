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

#include <utils/AssParse.hpp>
#include <utils/MusicInfo.hpp>

/**
 * @brief 歌词界面
 */
class LyricView : public QWidget {
    Q_OBJECT
public:
    explicit LyricView(QWidget* parent = nullptr);

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
};

#endif // !_HX_LYRIC_VIEW_H_