// #include <gtk/gtk.h>
// #include <gtk-layer-shell/gtk-layer-shell.h>

// 破烂qt!!!
// 老子 <gtk/gtk.h> 定义了 GDBusSignalInfo **signals;
// 你给我宏替换了!?
// 全tm给我用 Q_SIGNALS 宏
// 还拓展语法呢? 笑了

#include <window/LyricWindow.h>

#include <QHBoxLayout>
#include <QGuiApplication>

LyricWindow::LyricWindow(QWindow* parent)
    : QWindow(parent)
{
    // 为什么设置标题栏也会不能使用顶层?
    if (QGuiApplication::platformName() == "wayland") {
       _mainWidget = QWidget::createWindowContainer(this);
    } else {
        _mainWidget = new QWidget();
        _mainWidget->setWindowTitle("HX Music - Lyric");
        _mainWidget->setWindowFlags(
            _mainWidget->windowFlags()
            | Qt::FramelessWindowHint       // 无边框
            | Qt::WindowTransparentForInput // 鼠标穿透
        );
    }

    _mainWidget->setWindowFlags(
        Qt::WindowStaysOnTopHint // 顶层
    );

    _mainWidget->setContentsMargins(0, 0, 0, 0);

    _mainWidget->resize(800, 200);
    // _mainWidget->setAttribute(Qt::WA_Mapped);
    // _mainWidget->setAttribute(Qt::WA_TransparentForMouseEvents); // 保证窗口本身不会阻止鼠标事件传播
    _mainWidget->setAttribute(Qt::WA_TranslucentBackground); // 透明窗口

    auto* layout = new QHBoxLayout;
    _mainWidget->setLayout(layout);

    _lyricView = new LyricView(_mainWidget);
    layout->addWidget(_lyricView);
}

void LyricWindow::hideEvent(QHideEvent* event) {
    _windowPos = _mainWidget->pos();
}

void LyricWindow::showEvent(QShowEvent* event) {
    // 窗口显示时恢复之前保存的位置
    if (!_windowPos.isNull()) {
        _mainWidget->move(_windowPos);  // 恢复之前记录的位置
    }
    QWindow::showEvent(event);
    _lyricView->showSettingView();
}
