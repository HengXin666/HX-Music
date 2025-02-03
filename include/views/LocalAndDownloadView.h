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
#ifndef _HX_LOCAL_AND_DOWNLOAD_VIEW_H_
#define _HX_LOCAL_AND_DOWNLOAD_VIEW_H_

#include <QWidget>
#include <QTreeWidget>

/**
 * @brief 本地与下载界面
 */
class LocalAndDownloadView : public QWidget {
    Q_OBJECT

public:
    explicit LocalAndDownloadView(QWidget* parent = nullptr);

private:
    QTreeWidget* _tree = new QTreeWidget(this);
};

#endif // !_HX_LOCAL_AND_DOWNLOAD_VIEW_H_