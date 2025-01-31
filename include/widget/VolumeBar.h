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
#include <QVBoxLayout>

#include <QDebug>

/**
 * @brief 音量条: 点击是开关, 带有音量条鼠标悬浮
 */
class VolumeBar : public QWidget {
    Q_OBJECT

    class VolumeSlider : public QWidget {
        
    public:
        VolumeSlider(QWidget* parent = nullptr) 
            : QWidget(parent) 
        {
            setWindowFlags(Qt::FramelessWindowHint | Qt::Popup); // 无边框 + 悬浮窗

            _slider->setRange(0, 100);
            _slider->setValue(75);

            _textPercentage->setText(QString("%1%").arg(_slider->value()));

            QVBoxLayout* vBL = new QVBoxLayout(this);
            vBL->addWidget(_textPercentage);
            vBL->addWidget(_slider);
            setLayout(vBL);
        }

    protected:
        void focusOutEvent(QFocusEvent* event) override {
            hide(); // 失去焦点时自动隐藏
        }
    
    private:
        QSlider* _slider = new QSlider(Qt::Vertical, this);
        QLabel* _textPercentage = new QLabel(this);
    };

public:
    explicit VolumeBar(QWidget* parent = nullptr);

private:
    VolumeSlider* _slider = new VolumeSlider(this);
    QPushButton* _btn = new QPushButton(this);
};

#endif // !_HX_VOLUME_BAR_H_