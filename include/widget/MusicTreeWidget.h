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
#include <QStyledItemDelegate>
#include <QFileInfo>

class MultiLineItemDelegate : public QStyledItemDelegate {
    // 间距
    inline static constexpr int Padding = 5;

    // 图片大小
    inline static constexpr int ImageSize = 48;

    // 文本外边距
    inline static constexpr int TextMargin = 5;

    // 标题字体大小
    inline static constexpr int TitleFontSize = 10;

    // 歌手字体大小
    inline static constexpr int ArtistFontSize = 8;
public:
    MultiLineItemDelegate(QObject* parent = nullptr) 
        : QStyledItemDelegate(parent) 
    {}

    void paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const override;

    QSize sizeHint(
        const QStyleOptionViewItem& option, 
        const QModelIndex& index
    ) const override;
};

/**
 * @brief 音乐树状显示控件
 */
class MusicTreeWidget : public QTreeWidget {
    Q_OBJECT

    friend MultiLineItemDelegate;

    enum class ItemData : int {
        Title = 0,
        NodeType = 1,
        FilePath = 2,
        PlayQueue = 3,
    };

    enum class NodeType : unsigned int {
        File,
        Folder
    };

    /**
     * @brief 设置节点项类型
     * @param item 
     * @param type 
     */
    void setNodeType(QTreeWidgetItem* item, NodeType type) {
        item->setFlags(
            item->flags() 
            | Qt::ItemIsDragEnabled
            | Qt::ItemIsSelectable 
            | Qt::ItemIsEnabled
        );
        item->setData(
            static_cast<int>(ItemData::NodeType),
            Qt::UserRole,
            static_cast<unsigned int>(type)
        );
    }
    
    /**
     * @brief 获取节点项的类型
     * @param item 
     * @return NodeType 
     */
    NodeType getNodeType(QTreeWidgetItem* item) {
        return static_cast<NodeType>(
            item->data(
                static_cast<int>(ItemData::NodeType),
                Qt::UserRole
            ).toUInt()
        );
    }

public:
    explicit MusicTreeWidget(QWidget* parent = nullptr);

    /**
     * @brief 添加一个文件
     * @param fileInfo 文件信息
     * @param index 需要插入的位置
     * @param parentItem 父元素所属
     * @return true 添加成功
     * @return false 添加失败: 该元素不可播放/暂不支持
     */
    bool addFileItem(const QFileInfo& fileInfo, int index, QTreeWidgetItem* parentItem);

    /**
     * @brief 添加一个文件夹节点
     * @param fileInfo 
     * @param index 
     * @param parentItem 
     */
    void addFolderItem(const QFileInfo& fileInfo, int index, QTreeWidgetItem* parentItem);

    /**
     * @brief 更新`parentItem`代表的`一层`音频的编号
     * @param parentItem 需要更新的元素的父节点, 如果是顶层则为`nullptr`
     */
    void updateItemNumber(QTreeWidgetItem *parentItem);

protected:
    void resizeEvent(QResizeEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;

    void dragMoveEvent(QDragMoveEvent* event) override;

    void dropEvent(QDropEvent* event) override;

private:
    /**
     * @brief 确认拖放的位置
     * @param event 
     * @return std::pair<QTreeWidgetItem*, int> 该位置的父节点, 子节点索引
     */
    std::pair<QTreeWidgetItem*, int> determineDropPosition(QDropEvent* event);
};

#endif // !_HX_MUSIC_TREE_WIDGET_H_