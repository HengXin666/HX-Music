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
#ifndef _HX_PLAY_BAR_H_
#define _HX_PLAY_BAR_H_

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

#include <widget/TimeProgressText.h>
#include <widget/ScrollText.h>
#include <widget/VolumeBar.h>
#include <widget/SvgIconPushButton.h>

// 音频操作按钮 (仅设置样式)
class ActionsPushButton : public SvgIconPushButton {
    Q_OBJECT
public:
    explicit ActionsPushButton(
        const QString& svgPath, 
        QWidget* parent = nullptr
    )
        : SvgIconPushButton(
            svgPath,
            QColor("#990099"),
            QColor("RED"), 
            parent)
    {
        // 设置样式
        setStyleSheet(R"(
            QPushButton {
                background: transparent;
                border: none;
                border-radius: 12px;
            }
        )");

        setFixedSize(24, 24);
    }
};

/**
 * @brief 播放栏控件
 */
class PlayBar : public QWidget {
    Q_OBJECT
public:
    explicit PlayBar(QWidget* parent = nullptr);

private:
    // 进度条
    QProgressBar* _barPlayProgress = new QProgressBar(this);

    // 歌曲图片
    QLabel* _imgMusic = new QLabel(this);

    // 滚动歌曲信息
    ScrollText* _textMusicData = new ScrollText(this);

    // 喜欢/评论/下载/分享
    ActionsPushButton* _btnLike = new ActionsPushButton(
        ":/icons/like.svg", this);
    ActionsPushButton* _btnComment = new ActionsPushButton(
        ":/icons/message.svg", this);
    ActionsPushButton* _btnDownload = new ActionsPushButton(
        ":/icons/download.svg", this);
    ActionsPushButton* _btnShare = new ActionsPushButton(
        ":/icons/share.svg", this);

    // 播放时长文本
    TimeProgressText* _textTimeProgress = new TimeProgressText(this);

    // 上一首/暂停或继续/下一首
    QPushButton* _btnPrev = new QPushButton(this);
    QPushButton* _btnPlayPause = new QPushButton(this);
    QPushButton* _btnNext = new QPushButton(this);

    // 播放序列/音量大小/歌词
    QComboBox* _cbxPlayMode = new QComboBox(this);
    VolumeBar* _volumeBar = new VolumeBar(this);
    QPushButton* _btnLyric = new QPushButton(this);
};

#endif // !_HX_PLAY_BAR_H_