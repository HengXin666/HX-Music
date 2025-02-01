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

/**
 * @brief 悬浮音量条
 */
class VolumeSlider : public QWidget {
    Q_OBJECT
public:
    VolumeSlider(QWidget* parent = nullptr, QPushButton* btn = nullptr) 
        : QWidget(parent) 
    {
        setAttribute(Qt::WA_TranslucentBackground); // 设置没有窗体
        setWindowFlags(Qt::FramelessWindowHint | Qt::Popup); // 无边框 + 悬浮窗

        btn->setIcon(QIcon(":/icons/volume_up.svg"));

        _slider->setRange(0, 100);
        _slider->setValue(75);

        _textPercentage->setText(QString("%1%").arg(_slider->value()));

        QVBoxLayout* vBL = new QVBoxLayout(this);
        vBL->addWidget(_textPercentage);
        vBL->addWidget(_slider);
        setLayout(vBL);

        // 预先加载图标, 避免每次都加载文件
        QIcon vLowIcon(":/icons/volume_low.svg");
        QIcon vUpIcon(":/icons/volume_up.svg");
        QIcon vOffIcon(":/icons/volume_off.svg");

        // 拖动条值改变槽
        connect(_slider, &QSlider::valueChanged, this,
            [this, btn,
            vLowIcon = std::move(vLowIcon),
            vUpIcon = std::move(vUpIcon),
            vOffIcon = std::move(vOffIcon)
        ](
            int val
        ) {
            _textPercentage->setText(QString("%1%").arg(val));
            // 触发修改图标
            // 如果音量在之前的区间则不修改
            if (val >= 50) {
                btn->setIcon(vUpIcon); // 使用缓存的图标
            } else if (val > 0) {
                btn->setIcon(vLowIcon);
            } else {
                btn->setIcon(vOffIcon);
            }
            // @todo 是否需要使用命令模式 + 观察者模式/中介者模式 重构?
        });
    }

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