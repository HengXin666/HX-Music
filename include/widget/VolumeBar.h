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

#include <QPushButton>
#include <QSlider>
#include <QEvent>

/**
 * @brief 音量条: 点击是开关, 带有音量条鼠标悬浮
 */
class VolumeBar : public QPushButton {
    Q_OBJECT
public:
    explicit VolumeBar(QWidget* parent = nullptr);

protected:
    bool event(QEvent *event) override {
        if (event->type() == QEvent::Enter) {
            // 鼠标进入时显示音量条
            if (isChecked()) {
                _slider->setVisible(true);
            }
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开时隐藏音量条
            _slider->setVisible(false);
        }
        return QPushButton::event(event);
    }

private:
    QSlider* _slider = new QSlider(Qt::Vertical, this);
};

#endif // !_HX_VOLUME_BAR_H_