#include <widget/MainDisplayBar.h>

#include <QVBoxLayout>

MainDisplayBar::MainDisplayBar(QWidget* parent)
    : QWidget(parent)
{
    _btnPop->setIcon(QIcon{":/icons/back.svg"});
    _btnPop->setToolTip("回退");
    _btnPop->setFixedWidth(_btnPop->iconSize().width() + 10);
    QVBoxLayout* vBL = new QVBoxLayout(this);
    vBL->addWidget(_btnPop, 0, Qt::AlignTop);
    vBL->addWidget(_stackedWidget, 1);
}