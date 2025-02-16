#include <views/LyricView.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QWheelEvent>

#include <widget/AssLyricWidget.h>
#include <widget/SvgIconPushButton.h>
#include <utils/LayoutOperate.hpp>

LyricView::LyricView(QWidget* parent)
    : QWidget(parent)
{
    auto* vLayout = new QVBoxLayout(this);
    auto* settingWidget = new QWidget;
    settingWidget->setStyleSheet(R"(
        background-color:rgba(0, 0, 0, 0.2);
    )");
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
    auto* btnUnLock = new SvgIconPushButton(
        ":icons/unlock.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    btnUnLock->setToolTip("取消固定窗口");
    btnUnLock->setIconSize({32, 32});
    mainHLayout->addWidget(btnUnLock);
    btnUnLock->setHidden(true); // 默认不可见

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
        [this, parent, settingWidget, settingHLayout, btnUnLock]() {
        parent->setWindowFlags(
            parent->windowFlags()
            | Qt::FramelessWindowHint       // 无边框 
            | Qt::WindowTransparentForInput // 鼠标穿透
        );
        // 隐藏操作布局
        HX::LayoutOperate::setHidden(settingHLayout, true);
        btnUnLock->setHidden(false);

        settingWidget->setStyleSheet("");
        parent->show();
    });

    // 取消固定窗口
    connect(btnUnLock, &QPushButton::clicked, this,
        [this, parent, settingWidget, settingHLayout, btnUnLock]() {
        parent->setWindowFlags(
            parent->windowFlags()
            & ~Qt::FramelessWindowHint       // 无边框 
            & ~Qt::WindowTransparentForInput // 鼠标穿透
        );

        // 显示操作布局
        HX::LayoutOperate::setHidden(settingHLayout, false);
        btnUnLock->setHidden(true);

        settingWidget->setStyleSheet(R"(
            background-color:rgba(0, 0, 0, 0.2);
        )");
        parent->show();
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
        constexpr int Add = 10; // 滚动增量 (@todo 后期可以尝试配置到设置? 没必要吧)
        // 获取滚轮的增量值
        // 判断滚轮的方向: 
        // 如果是向上滚动, delta > 0
        // 向下滚动, delta < 0
        if (event->angleDelta().y() > 0) {
            // 向上滚, 增加大小
            _lyricWidget->resize(
                _lyricWidget->width() + Add,
                _lyricWidget->height() + Add
            );
        } else {
            // 向下滚, 减小大小
            _lyricWidget->resize(
                _lyricWidget->width() - Add,
                _lyricWidget->height() - Add
            );
        }
    }
}