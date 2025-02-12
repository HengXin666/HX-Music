#include <window/MainWindow.h>

#include <QGridLayout>
#include <QImage>

#include <widget/PlayBar.h>
#include <widget/Sidebar.h>
#include <widget/MainDisplayBar.h>
#include <widget/TopBar.h>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("HX Music");
    // setCentralWidget(new QWidget);

    // 配置为无边窗口 | Qt::FramelessWindowHint
    // Arch本机不可用
    // setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    // 设置窗口大小
    resize(800, 600);

    // 网格布局
    QGridLayout* gL = new QGridLayout(this);

    // logo
    QImage logoImg = QImage(":/logo/HXMusic_logo.png");
    QLabel* _logo = new QLabel(this);
    _logo->setPixmap(QPixmap::fromImage(logoImg).scaled(
        160, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _logo->setGeometry(logoImg.rect());
    _logo->setFixedSize({160, 50});
    gL->addWidget(_logo, 0, 0, 1, 1);

    // 侧边栏
    Sidebar* _leftSidebar = new Sidebar(this);
    _leftSidebar->setFixedWidth(160);
    _leftSidebar->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Expanding});
    gL->addWidget(_leftSidebar, 1, 0, 1, 1);

    // 顶部栏
    TopBar* _topBal = new TopBar(this);
    gL->addWidget(_topBal, 0, 1, 1, -1);

    // 中间正文
    MainDisplayBar* _mainDisplayBar = new MainDisplayBar(this);
    gL->addWidget(_mainDisplayBar, 1, 1, 1, -1);

    // 底部播放栏
    PlayBar* _playBar = new PlayBar(this);
    gL->addWidget(_playBar, 2, 0, 1, -1);

    setLayout(gL);
}