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
#include <QStyledItemDelegate>
#include <QPainter>

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

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (index.column() == 0) {
            // 固定字体尺寸计算
            QFont titleFont = option.font;
            titleFont.setPointSize(TitleFontSize);
            QFont artistFont = titleFont;
            artistFont.setPointSize(ArtistFontSize);
            
            const int textHeight = QFontMetrics(titleFont).height() 
                                 + QFontMetrics(artistFont).height() 
                                 + 2; // 行间距
            
            // 取图片高度和文字高度的较大值
            const int totalHeight = qMax(ImageSize, textHeight) + 2 * Padding;
            
            return QSize(200, totalHeight); // 适当宽度
        }
        return QStyledItemDelegate::sizeHint(option, index);
    }
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
    void setNodeType(QTreeWidgetItem *item, NodeType type) {
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
    NodeType getNodeType(QTreeWidgetItem *item) {
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
    bool addFileItem(const QFileInfo &fileInfo, int index, QTreeWidgetItem *parentItem);

    /**
     * @brief 添加一个文件夹节点
     * @param fileInfo 
     * @param index 
     * @param parentItem 
     */
    void addFolderItem(const QFileInfo &fileInfo, int index, QTreeWidgetItem *parentItem);

    /**
     * @brief 更新`parentItem`代表的`一层`音频的编号
     * @param parentItem 需要更新的元素的父节点, 如果是顶层则为`nullptr`
     */
    void updateItemNumber(QTreeWidgetItem *parentItem);

protected:
    void resizeEvent(QResizeEvent *event) override {
        QTreeWidget::resizeEvent(event); // 调用父类

        int totalWidth = width(); // 减去固定列的宽度
        setColumnWidth(0, totalWidth * 0.5);
        setColumnWidth(1, totalWidth * 0.25);
        setColumnWidth(2, totalWidth * 0.15);
        setColumnWidth(3, totalWidth * 0.1);
    }

    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->source() == this) {
            event->accept();
            return;
        } else if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
        QTreeWidget::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent *event) override {
        if (event->source() == this) {
            event->accept();
        } else if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
        QTreeWidget::dragMoveEvent(event);
    }

    void dropEvent(QDropEvent *event) override;

private:
    /**
     * @brief 确认拖放的位置
     * @param event 
     * @return std::pair<QTreeWidgetItem*, int> 该位置的父节点, 子节点索引
     */
    std::pair<QTreeWidgetItem*, int> determineDropPosition(QDropEvent *event) {
        QTreeWidgetItem* targetItem = itemAt(event->position().toPoint());
        QAbstractItemView::DropIndicatorPosition indicatorPos = dropIndicatorPosition();
        QTreeWidgetItem* parentItem = nullptr;
        int insertRow = 0;

        // 如果在空白区域, 直接插入到根节点的末尾
        if (!targetItem) {
            parentItem = invisibleRootItem();
            insertRow = parentItem->childCount();
        } else {
            // 如果拖放指示器不在目标项上, 而是在其上方或下方, 则需要计算目标插入位置
            if (indicatorPos == QAbstractItemView::AboveItem
                || indicatorPos == QAbstractItemView::BelowItem) {
                parentItem = targetItem->parent();
                if (!parentItem) {
                    parentItem = invisibleRootItem();
                }
                insertRow = parentItem->indexOfChild(targetItem);
                if (indicatorPos == QAbstractItemView::BelowItem) {
                    ++insertRow; // 下方则在目标项后插入
                }
            } else {
                // 当指示器位于目标项正中, 且目标项是文件夹时, 将作为子项插入
                if (getNodeType(targetItem) == NodeType::Folder) {
                    parentItem = targetItem;
                    insertRow = targetItem->childCount();
                } else {
                    // 否则仍以目标项所在父项为准
                    parentItem = targetItem->parent();
                    if (!parentItem) {
                        parentItem = invisibleRootItem();
                    }
                    insertRow = parentItem->indexOfChild(targetItem);
                }
            }
        }
        return {parentItem, insertRow};
    }

    QList<QTreeWidgetItem*> _list;
};

#endif // !_HX_MUSIC_TREE_WIDGET_H_