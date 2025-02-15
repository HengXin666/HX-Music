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

// #include <KWindowSystem>
// #include <NETWM>

#include <views/LyricView.h>

LyricWindow::LyricWindow(QWindow* parent)
    : QWindow(parent)
{
    setTitle("HX Music - Lyric");

    _mainWidget->setWindowFlags(
        // Qt::FramelessWindowHint // 无边框
        // | 
        Qt::WindowStaysOnTopHint    // 顶层
    );
    
    // QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    _mainWidget->setMinimumSize(800, 600);
    _mainWidget->setAttribute(Qt::WA_TranslucentBackground);

    auto* layout = new QHBoxLayout;
    _mainWidget->setLayout(layout);

    auto* view = new LyricView(_mainWidget);
    layout->addWidget(view);
}