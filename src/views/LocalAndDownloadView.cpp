#include <views/LocalAndDownloadView.h>

#include <QVBoxLayout>

LocalAndDownloadView::LocalAndDownloadView(QWidget* parent)
    : QWidget(parent) 
{
    QVBoxLayout* vBL = new QVBoxLayout(this);
    vBL->addWidget(_tree);
    
    _tree->setHeaderLabels({
        "计数",
        "图片",
        "名称",
        "专辑",
        "时长",
        "大小"
    });

    setLayout(vBL);
}