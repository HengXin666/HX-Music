#include <views/LyricView.h>

#include <QWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QHideEvent>

#include <singleton/SignalBusSingleton.h>
#include <widget/AssLyricWidget.h>
#include <utils/LayoutOperate.hpp>

LyricView::LyricView(QWidget* parent)
    : QWidget(parent)
{
    auto* vLayout = new QVBoxLayout(this);
    auto* settingWidget = new QWidget;
    vLayout->addWidget(settingWidget);
    auto* mainHLayout = new QHBoxLayout(settingWidget);
    mainHLayout->addStretch();
    auto* settingHLayout = new QHBoxLayout;
    mainHLayout->addLayout(settingHLayout);

    // 偏移 -0.5s
    auto* btnNegativeOffset = new SvgIconPushButton(
        ":icons/back.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    btnNegativeOffset->setToolTip("歌词偏移 -0.5s");
    btnNegativeOffset->setIconSize({32, 32});
    settingHLayout->addWidget(btnNegativeOffset);

    // 偏移 +0.5s
    auto* btnPositiveOffset = new SvgIconPushButton(
        ":icons/enter.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    btnPositiveOffset->setToolTip("歌词偏移 +0.5s");
    btnPositiveOffset->setIconSize({32, 32});
    settingHLayout->addWidget(btnPositiveOffset);

    // 固定位置
    auto* btnLock = new SvgIconPushButton(
        ":icons/lock.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    btnLock->setToolTip("固定歌词悬浮窗口");
    btnLock->setIconSize({32, 32});
    settingHLayout->addWidget(btnLock);

    // 取消固定位置
    _btnUnLock = new SvgIconPushButton(
        ":icons/unlock.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    _btnUnLock->setToolTip("取消固定窗口");
    _btnUnLock->setIconSize({32, 32});
    mainHLayout->addWidget(_btnUnLock);
    _btnUnLock->setHidden(true); // 默认不可见

    // 移动字幕位置
    auto* btnMove = new SvgIconPushButton(
        ":icons/move.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    btnMove->setToolTip("移动字幕位置");
    btnMove->setIconSize({32, 32});
    settingHLayout->addWidget(btnMove);

    // 字幕水平居中
    auto* btnCenter = new SvgIconPushButton(
        ":icons/center.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    btnCenter->setToolTip("字幕水平居中");
    btnCenter->setIconSize({32, 32});
    settingHLayout->addWidget(btnCenter);

    mainHLayout->addStretch();

    // ass歌词渲染
    // 创建一个垂直间隔项, 占据 _lyricWidget 的空间
    vLayout->addItem(new QSpacerItem(_lyricWidget->width(), _lyricWidget->height(),
    QSizePolicy::Expanding, QSizePolicy::Expanding));
    _lyricWidget->move(0, 0); // _lyricWidget 就不受布局了

    // 偏移 -0.5s 槽函数
    connect(btnNegativeOffset, &QPushButton::clicked, this,
        [this]() {
        _lyricWidget->addOffset(-500);
    });

    // 偏移 +0.5s 槽函数
    connect(btnPositiveOffset, &QPushButton::clicked, this,
        [this]() {
        _lyricWidget->addOffset(500);
    });

    // 固定窗口
    connect(btnLock, &QPushButton::clicked, this,
        [this, parent, settingWidget, settingHLayout]() {
        parent->windowHandle()->setFlags(
            parent->windowHandle()->flags()
            | Qt::FramelessWindowHint       // 无边框 
            | Qt::WindowTransparentForInput // 鼠标穿透
        );
        // 隐藏操作布局
        HX::LayoutOperate::setHidden(settingHLayout, true);

        // 设置窗口内容区域的边距
        // @todo 此处是硬编码了, 目前 Wayland 合适
        // 用于平衡有边窗口到无边窗口的位移, 能力/精力有限 fuck you Wayland!
        // 怎么这么多不行? 如果真希望可以, 那只能搞一个支持鼠标事件的无边窗口了!! so is todo bro!!
        parent->setContentsMargins(
            QMargins(4,24,4,4)
        );

        SignalBusSingleton::get().lyricViewLockChanged(_isLock = true);
        parent->windowHandle()->requestUpdate();
    });

    // 取消固定窗口
    connect(_btnUnLock, &QPushButton::clicked, this,
        [this, parent, settingWidget, settingHLayout]() {
        parent->windowHandle()->setFlags(
            parent->windowHandle()->flags()
            & ~Qt::FramelessWindowHint       // 无边框 
            & ~Qt::WindowTransparentForInput // 鼠标穿透
        );

        // 显示操作布局
        HX::LayoutOperate::setHidden(settingHLayout, false);
        _btnUnLock->setHidden(true);

        parent->setContentsMargins(
            QMargins(0,0,0,0)
        );

        SignalBusSingleton::get().lyricViewLockChanged(_isLock = false);
    });

    // 启用/关闭: 移动字幕位置, 并且高亮字幕渲染的矩形范围
    connect(btnMove, &QPushButton::clicked, this,
        [this, btnMove] {
        _isMove = !_isMove;
        if (_isMove) {
            btnMove->showHoverIcon();
            _lyricWidget->setMoveFlag(true);
        } else {
            btnMove->showOrdinaryIcon();
            _lyricWidget->setMoveFlag(false);
        }
        update();
    });

    // 字幕水平居中
    connect(btnCenter, &QPushButton::clicked, this,
        [this]() {
        _lyricWidget->move((width() - _lyricWidget->width()) / 2 , _lyricWidget->pos().y());
    });

    setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            border-radius: 12px;
        }
    )");
}

void LyricView::showSettingView() {
    _btnUnLock->click();
}

void LyricView::mousePressEvent(QMouseEvent* event) {
    if (_isMove && event->button() == Qt::LeftButton) {
        auto rect = _lyricWidget->rect();;
        rect.translate(_lyricWidget->pos());
        if (rect.contains(event->pos())) {
            _relativePos = event->globalPosition().toPoint() - _lyricWidget->pos();
        }
    }
}

void LyricView::mouseMoveEvent(QMouseEvent* event) {
    if (_isMove && (event->buttons() & Qt::LeftButton)) {
        _lyricWidget->move(event->globalPosition().toPoint() - _relativePos);
    }
}

void LyricView::wheelEvent(QWheelEvent* event) {
    if (_isMove) {
        constexpr double ScaleFactor = 1.05;  // 5% 的缩放增量

        int newWidth, newHeight;
        if (event->angleDelta().y() > 0) {
            // 放大
            newWidth = _lyricWidget->width() * ScaleFactor;
        } else {
            // 缩小
            newWidth = _lyricWidget->width() / ScaleFactor;
        }

        // 根据 16:9 比例计算新高度
        newHeight = newWidth * 9 / 16;

        _lyricWidget->resize(newWidth, newHeight);
    }
}