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
#ifndef _HX_MUSIC_TREE_WIDGET_H_
#define _HX_MUSIC_TREE_WIDGET_H_

#include <QTreeWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>

/**
 * @brief 音乐树状显示控件
 */
class MusicTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit MusicTreeWidget(QWidget* parent = nullptr);

    /**
     * @brief 添加一个文件
     * @param fileInfo 文件信息
     * @param parentItem 父元素所属
     * @return true 添加成功
     * @return false 添加失败: 该元素不可播放/暂不支持
     */
    bool addFileItem(const QFileInfo &fileInfo, QTreeWidgetItem *parentItem) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, fileInfo.fileName());
        item->setText(1, QString::number(fileInfo.size()));
        item->setText(2, fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        // 根据需要设置图标，这里假设你已经有相应的图标资源
        item->setIcon(0, QIcon(":/icon/file.png"));
        
        // 将新项添加到某个目标文件夹项中或作为顶层项
        // 如果拖拽到某个文件夹项上，可以进行判断并添加为其子项
        // 例如:
        if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "folder") {
            parentItem->addChild(item);
            parentItem->setExpanded(true);
        } else {
            addTopLevelItem(item);
        }
        return true;
    }

    void addFolderItem(const QFileInfo &fileInfo, QTreeWidgetItem *parentItem) {
        namespace fs = std::filesystem;

        // 1. 新建一个目录元素
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, fileInfo.fileName());
        item->setData(0, Qt::UserRole, "folder");

        std::size_t cnt = 0;

        if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "folder") {
            parentItem->addChild(item);
            parentItem->setExpanded(true);
        } else {
            addTopLevelItem(item);
        }

        // 2. 递归遍历文件夹, 并且构建
        for (auto const& it : fs::directory_iterator{
            fileInfo.filesystemFilePath()
        }) {
            if (it.is_directory()) { // 是文件夹
                addFolderItem(QFileInfo{it.path()}, item);
            } else {
                cnt += addFileItem(QFileInfo{it.path()}, item);
            }
        }
    }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        // 判断是否包含文件URL数据
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent *event) override {
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dropEvent(QDropEvent *event) override {
        const QMimeData *mimeData = event->mimeData();
        if (mimeData->hasUrls()) {
            QList<QUrl> urlList = mimeData->urls();
            // 遍历所有拖入的URL
            for (const QUrl &url : urlList) {
                // 转为本地文件路径
                QString localPath = url.toLocalFile();
                if (!localPath.isEmpty()) {
                    QFileInfo fileInfo(localPath);
                    if (fileInfo.exists()) {
                        // 判断是文件夹还是文件
                        if (fileInfo.isDir()) {
                            // 这里处理拖入的文件夹
                            addFolderItem(fileInfo, itemAt(mapFromGlobal(QCursor::pos())));
                        } else if (fileInfo.isFile()) {
                            // 处理拖入的文件
                            addFileItem(fileInfo, itemAt(mapFromGlobal(QCursor::pos())));
                        }
                    }
                }
            }
            event->acceptProposedAction();
        } else {
            // 如果不是文件URL数据，调用默认处理
            QTreeWidget::dropEvent(event);
        }
    }
};

#endif // !_HX_MUSIC_TREE_WIDGET_H_