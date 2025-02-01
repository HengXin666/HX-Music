#include <window/MainWindow.h>

#include <QGridLayout>

#include <widget/PlayBar.h>
#include <widget/Sidebar.h>
#include <widget/MainDisplayBar.h>

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
    Sidebar* _leftSidebar = new Sidebar(this);
    _leftSidebar->setFixedWidth(160);
    _leftSidebar->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Expanding});
    gL->addWidget(_leftSidebar, 1, 0, 1, 1);

    // 顶部栏

    // 中间正文
    MainDisplayBar* _mainDisplayBar = new MainDisplayBar(this);
    gL->addWidget(_mainDisplayBar, 1, 1, 1, -1);

    // 底部播放栏
    PlayBar* _playBar = new PlayBar(this); 
    gL->addWidget(_playBar, 2, 0, 1, -1);

    setLayout(gL);
}