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
#ifndef _HX_VOLUME_BAR_H_
#define _HX_VOLUME_BAR_H_

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QEvent>

/**
 * @brief 悬浮音量条
 */
class VolumeSlider : public QWidget {
    Q_OBJECT
public:
    VolumeSlider(QWidget* parent = nullptr, QPushButton* btn = nullptr);

private:
    QSlider* _slider = new QSlider(Qt::Vertical, this);
    QLabel* _textPercentage = new QLabel(this);
};

/**
 * @brief 音量条: 点击是开关, 带有音量条鼠标悬浮
 */
class VolumeBar : public QWidget {
    Q_OBJECT
public:
    explicit VolumeBar(QWidget* parent = nullptr);

private:
    QPushButton* _btn = new QPushButton(this);
    VolumeSlider* _slider = new VolumeSlider(this, _btn);
};

#endif // !_HX_VOLUME_BAR_H_