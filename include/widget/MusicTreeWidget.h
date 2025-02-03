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
    bool addFileItem(const QFileInfo &fileInfo, int index, QTreeWidgetItem *parentItem) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, fileInfo.fileName());
        item->setText(1, QString::number(fileInfo.size()));
        item->setText(2, fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        // 根据需要设置图标，这里假设你已经有相应的图标资源
        item->setIcon(0, QIcon(":/icon/file.png"));
        
        // 使用当前 flags 但移除 Qt::ItemIsDropEnabled
        Qt::ItemFlags flags = item->flags();
        flags &= ~Qt::ItemIsDropEnabled;
        // 保留拖动、可选、可用标志
        flags |= (Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setFlags(flags);

        // 将新项添加到某个目标文件夹项中或作为顶层项
        // 如果拖拽到某个文件夹项上，可以进行判断并添加为其子项
        // 例如:
        if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "folder") {
            parentItem->insertChild(index, item);
            parentItem->setExpanded(true);
        } else {
            addTopLevelItem(item);
        }
        return true;
    }

    void addFolderItem(const QFileInfo &fileInfo, int index, QTreeWidgetItem *parentItem) {
        namespace fs = std::filesystem;

        // 1. 新建一个目录元素
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, fileInfo.fileName());
        item->setData(0, Qt::UserRole, "folder");

        // 允许拖放
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        std::size_t cnt = 0;

        if (parentItem && parentItem->data(0, Qt::UserRole).toString() == "folder") {
            parentItem->insertChild(index, item);
            parentItem->setExpanded(true);
        } else {
            addTopLevelItem(item);
        }

        // 2. 递归遍历文件夹, 并且构建
        int i = 0;
        for (auto const& it : fs::directory_iterator{
            fileInfo.filesystemFilePath()
        }) {
            if (it.is_directory()) { // 是文件夹
                addFolderItem(QFileInfo{it.path()}, i, item);
            } else {
                cnt += addFileItem(QFileInfo{it.path()}, i, item);
            }
            ++i;
        }
    }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->source() == this) {
            event->accept();
            return;
        }
        // 判断是否包含文件URL数据
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent *event) override {
        if (event->source() == this) {
            event->accept();
            return;
        }
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dropEvent(QDropEvent *event) override {
        // 内部拖拽交由默认实现处理
        if (event->source() == this) {
            QTreeWidget::dropEvent(event);
            return;
        }
        
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls()) {
            QTreeWidgetItem* targetItem = itemAt(event->position().toPoint());
            QAbstractItemView::DropIndicatorPosition indicatorPos = dropIndicatorPosition();

            QTreeWidgetItem* parentItem = nullptr;
            int insertRow = 0;

            // 如果在空白区域，直接插入到根节点的末尾
            if (!targetItem) {
                parentItem = invisibleRootItem();
                insertRow = parentItem->childCount();
            } else {
                // 如果拖放指示器不在目标项上，而是在其上方或下方，则需要计算目标插入位置
                if (indicatorPos == QAbstractItemView::AboveItem ||
                    indicatorPos == QAbstractItemView::BelowItem) {
                    parentItem = targetItem->parent();
                    if (!parentItem)
                        parentItem = invisibleRootItem();
                    insertRow = parentItem->indexOfChild(targetItem);
                    if (indicatorPos == QAbstractItemView::BelowItem)
                        ++insertRow; // 下方则在目标项后插入
                } else {
                    // 当指示器位于目标项正中，且目标项是文件夹时，将作为子项插入
                    if (targetItem->data(0, Qt::UserRole).toString() == "folder") {
                        parentItem = targetItem;
                        insertRow = targetItem->childCount();
                    } else {
                        // 否则仍以目标项所在父项为准
                        parentItem = targetItem->parent();
                        if (!parentItem)
                            parentItem = invisibleRootItem();
                        insertRow = parentItem->indexOfChild(targetItem);
                    }
                }
            }

            // 遍历所有拖入的 URL, 依次插入到计算好的位置
            for (const QUrl &url : mimeData->urls()) {
                QString localPath = url.toLocalFile();
                if (!localPath.isEmpty()) {
                    QFileInfo fileInfo(localPath);
                    if (fileInfo.exists()) {
                        if (fileInfo.isDir()) {
                            addFolderItem(fileInfo, insertRow, parentItem);
                        } else if (fileInfo.isFile()) {
                            addFileItem(fileInfo, insertRow, parentItem);
                        }
                        ++insertRow;
                    }
                }
            }
            event->acceptProposedAction();
        } else {
            QTreeWidget::dropEvent(event);
        }
    }
};

#endif // !_HX_MUSIC_TREE_WIDGET_H_