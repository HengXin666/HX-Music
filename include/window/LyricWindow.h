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
#ifndef _HX_LYRIC_WINDOW_H_
#define _HX_LYRIC_WINDOW_H_

#include <QWidget>
#include <QWindow>

#include <views/LyricView.h>

class LyricWindow : public QWindow {
    Q_OBJECT
public:
    explicit LyricWindow(QWindow* parent = nullptr);

    QWidget* getMainWidget() {
        return _mainWidget;
    }

    bool isLock() const {
        return _lyricView->isLock();
    }

    void showSettingView() {
        _lyricView->showSettingView();
    }

protected:
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    QWidget* _mainWidget = QWidget::createWindowContainer(this);
    LyricView* _lyricView;
    QPoint _windowPos;      // 窗口在屏幕上的位置
};

#endif // !_HX_LYRIC_WINDOW_H_