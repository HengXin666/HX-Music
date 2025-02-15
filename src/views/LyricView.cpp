#include <views/LyricView.h>

#include <widget/AssLyricWidget.h>

#include <QHBoxLayout>
#include <QVBoxLayout>

LyricView::LyricView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    QHBoxLayout* settingHLayout = new QHBoxLayout;
    vLayout->addLayout(settingHLayout);
    auto* lyricView = new AssLyricWidget(this);
    vLayout->addWidget(lyricView);
}