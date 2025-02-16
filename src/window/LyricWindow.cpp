// #include <gtk/gtk.h>
// #include <gtk-layer-shell/gtk-layer-shell.h>

// 破烂qt!!!
// 老子 <gtk/gtk.h> 定义了 GDBusSignalInfo **signals;
// 你给我宏替换了!?
// 全tm给我用 Q_SIGNALS 宏
// 还拓展语法呢? 笑了

#include <window/LyricWindow.h>

#include <QHBoxLayout>
#include <QApplication>

#include <views/LyricView.h>

LyricWindow::LyricWindow(QWindow* parent)
    : QWindow(parent)
{
    setTitle("HX Music - Lyric");

    _mainWidget->setWindowFlags(
        Qt::WindowStaysOnTopHint // 顶层
    );


    
    _mainWidget->resize(800, 200);
    _mainWidget->setAttribute(Qt::WA_TranslucentBackground); // 透明窗口

    auto* layout = new QHBoxLayout;
    _mainWidget->setLayout(layout);

    auto* view = new LyricView(_mainWidget);
    layout->addWidget(view);
}