#include <widget/MusicTreeWidget.h>

#include <QMenu>

MusicTreeWidget::MusicTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setStyleSheet("QTreeWidget::item { "
        "border-color: green;"
        "margin: 10px;"
    " }");

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    // 点击节点
    connect(this, &QTreeWidget::itemClicked, this, 
        [this](QTreeWidgetItem* item, int column) {
        if (getNodeType(item) == NodeType::Folder) {
            // 文件夹操作
            qDebug() << "点击的是文件夹：" << item->text(0);
        } else {
            // 播放音乐
            qDebug() << "点击的是文件：" << item->text(0);
        }
    });

    // 展开节点
    connect(this, &QTreeWidget::itemExpanded, this, 
        [this](QTreeWidgetItem *item) {
        item->setIcon(1, QIcon{":/icons/folder-open.svg"});
    });

    // 关闭节点
    connect(this, &QTreeWidget::itemCollapsed, this, 
        [this](QTreeWidgetItem *item) {
        item->setIcon(1, QIcon{":/icons/folder-close.svg"});
    });

    // 在构造函数中设置上下文菜单策略并连接信号
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested, this,
        [this](const QPoint& pos) {
        // 根据点击位置获取对应的节点
        QTreeWidgetItem *item = itemAt(pos);
        if (!item)
            return;  // 如果没有点击到有效节点，可以直接返回

        // 创建菜单，并添加所需动作
        QMenu menu;
        QAction *openAction = menu.addAction("打开");
        QAction *deleteAction = menu.addAction("删除");
        QAction *propertiesAction = menu.addAction("属性");

        // 将本地坐标转换为全局坐标，显示菜单
        QAction *selectedAction = menu.exec(mapToGlobal(pos));

        if (selectedAction == openAction) {
            qDebug() << "选择了打开：" << item->text(0);
        } else if (selectedAction == deleteAction) {
            qDebug() << "选择了删除：" << item->text(0);
        } else if (selectedAction == propertiesAction) {
            qDebug() << "选择了属性：" << item->text(0);
        }
    });
}