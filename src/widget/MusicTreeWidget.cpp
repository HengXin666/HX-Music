#include <widget/MusicTreeWidget.h>

MusicTreeWidget::MusicTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    // 允许[外部 + 内部]拖拽
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true); // 显示拖拽时的指示标志
    setDragDropMode(QAbstractItemView::DragDrop);
}