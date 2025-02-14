#include <gtk/gtk.h>
#include <gtk-layer-shell/gtk-layer-shell.h>

// 破烂qt!!!
// 老子 <gtk/gtk.h> 定义了 GDBusSignalInfo **signals;
// 你给我宏替换了!?
// 全tm给我用 Q_SIGNALS 宏
// 还拓展语法呢? 笑了

#include <window/LyricWindow.h>

#include <QWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>

#include <views/LyricView.h>

LyricWindow::LyricWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("HX Music - Lyric");

    // Wayland 兼容的窗口标志组合
    setWindowFlags(
        Qt::FramelessWindowHint  // 无边框
        | Qt::WindowStaysOnTopHint       // 强制置顶
        | Qt::ToolTip                    // 无任务栏条目且不获取焦点
        // | Qt::NoDropShadowWindowHint       // 可选：禁用阴影
    );
    
    // 关键属性设置
    setAttribute(Qt::WA_TranslucentBackground); // 必须启用透明背景
    setAttribute(Qt::WA_ShowWithoutActivating); // 显示时不激活
    setAttribute(Qt::WA_X11DoNotAcceptFocus, true); // 即使XWayland也禁用焦点
    
    // 输入处理
    setFocusPolicy(Qt::NoFocus);
    
    // Wayland 特定扩展（需要包含头文件）
#ifdef Q_OS_LINUX
    if (QGuiApplication::platformName() == "wayland") {
        auto windowHandle = window()->windowHandle();
        if (windowHandle) {
            // 设置窗口层级为覆盖层（需要合成器支持）
            windowHandle->setProperty("_q_wayland_layer", "overlay");
            // 禁用所有输入（点击穿透）
            windowHandle->setProperty("_q_wayland_accept_touch", false);
            windowHandle->setProperty("_q_wayland_accept_mouse", false);
        }
    }
#endif

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    QHBoxLayout* settingHLayout = new QHBoxLayout;
    vLayout->addLayout(settingHLayout);
    auto* lyricView = new LyricView(this);
    vLayout->addWidget(lyricView);
}

void LyricWindow::mousePressEvent(QMouseEvent* event) {
    event->ignore();
}