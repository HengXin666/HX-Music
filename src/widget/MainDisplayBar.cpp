#include <widget/MainDisplayBar.h>

#include <QVBoxLayout>

#include <singleton/GlobalSingleton.hpp>

MainDisplayBar::MainDisplayBar(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* vBL = new QVBoxLayout(this);
    vBL->addWidget(_stackedWidget, 1);

    GlobalSingleton::get().imp.setMainDisplayBar(this);
}