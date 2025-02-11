#include <views/LocalAndDownloadView.h>

#include <QVBoxLayout>

LocalAndDownloadView::LocalAndDownloadView(QWidget* parent)
    : QWidget(parent) 
{
    QVBoxLayout* vBL = new QVBoxLayout(this);
    vBL->addWidget(_tree);

    // 设置交替项颜色
    _tree->setAlternatingRowColors(true);
    // _tree->setPalette(QPalette(Qt::gray));
    // _tree->setStyleSheet(
    //     "QTreeWidget {"
    //     "    border: none;"
    //     "    background:rgb(0, 0, 0);"
    //     "}"
    //     "QTreeWidget::item {"
    //     "    height: 40px;"
    //     "    color: #333333;"
    //     "}"
    //     "QTreeWidget::item:selected {"
    //     "    background:rgb(138, 10, 130);"
    //     "}"
    //     "QTreeWidget::branch:closed:has-children {"
    //     "    image: url(:/icon/folder_closed.png);"
    //     "}"
    //     "QTreeWidget::branch:open:has-children {"
    //     "    image: url(:/icon/folder_open.png);"
    //     "}"
    // );

    setLayout(vBL);
}