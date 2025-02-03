#include <widget/MusicTreeWidget.h>

MusicTreeWidget::MusicTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setDragEnabled(true);                               // 允许拖动
    setAcceptDrops(true);                                   // 允许接受拖放
    setDefaultDropAction(Qt::MoveAction);           // 默认移动
    setDragDropMode(QAbstractItemView::InternalMove); // 允许[外部 + 内部]拖拽
    setDropIndicatorShown(true);                        // 高亮显示插入位置
}