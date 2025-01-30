#include <window/MainWindow.h>

#include <QGridLayout>

#include <widget/PlayBar.h>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("HX Music");
    // setCentralWidget(new QWidget);

    // 配置为无边窗口 | Qt::FramelessWindowHint
    // Arch本机不可用
    // setWindowFlags(Qt::Window);

    // 设置窗口大小
    resize(800, 600);

    // 网格布局
    QGridLayout* gL = new QGridLayout(this);

    // 侧边栏

    // 顶部栏

    // 中间正文

    // 底部播放栏
    PlayBar* _playBar = new PlayBar(this); 
    gL->addWidget(_playBar, 1, 0, 1, 3);

    setLayout(gL);
}