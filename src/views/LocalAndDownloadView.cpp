#include <views/LocalAndDownloadView.h>

#include <QVBoxLayout>

LocalAndDownloadView::LocalAndDownloadView(QWidget* parent)
    : QWidget(parent) 
{
    QVBoxLayout* vBL = new QVBoxLayout(this);
    vBL->addWidget(_tree);
    
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


    _tree->setHeaderLabels({
        "",
        "名称",
        "专辑",
        "时长",
        "大小"
    });

    setLayout(vBL);
}