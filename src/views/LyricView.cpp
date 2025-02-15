#include <views/LyricView.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QWheelEvent>

#include <widget/AssLyricWidget.h>
#include <widget/SvgIconPushButton.h>

LyricView::LyricView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    QHBoxLayout* settingHLayout = new QHBoxLayout;
    settingHLayout->addStretch();
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

    // 启用/关闭: 移动字幕位置
    connect(btnMove, &QPushButton::clicked, this,
        [this, btnMove]{
        _isMove = !_isMove;
        if (_isMove) {
            btnMove->showHoverIcon();
        } else {
            btnMove->showOrdinaryIcon();
        }
    });

    settingHLayout->addStretch();
    vLayout->addLayout(settingHLayout);

    // ass歌词渲染
    // 创建一个垂直间隔项, 占据 _lyricWidget 的空间
    vLayout->addItem(new QSpacerItem(_lyricWidget->width(), _lyricWidget->height(),
    QSizePolicy::Expanding, QSizePolicy::Expanding));
    _lyricWidget->move(0, 0); // _lyricWidget 就不受布局了

    // 偏移 -0.5s 槽函数
    connect(btnNegativeOffset, &QPushButton::clicked, this,
        [this](){
        _lyricWidget->addOffset(-500);
    });

    // 偏移 +0.5s 槽函数
    connect(btnPositiveOffset, &QPushButton::clicked, this,
        [this](){
        _lyricWidget->addOffset(500);
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