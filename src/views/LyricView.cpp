#include <views/LyricView.h>

#include <widget/AssLyricWidget.h>

#include <QHBoxLayout>
#include <QVBoxLayout>

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
    settingHLayout->addWidget(btnNegativeOffset);

    // 偏移 +0.5s
    auto* btnPositiveOffset = new SvgIconPushButton(
        ":icons/enter.svg",
        QColor{"#990099"},
        QColor{"red"},
        this
    );
    settingHLayout->addWidget(btnPositiveOffset);
    settingHLayout->addStretch();

    vLayout->addLayout(settingHLayout);
    auto* lyricView = new AssLyricWidget(this);
    vLayout->addWidget(lyricView);

    btnNegativeOffset->setIconSize({32, 32});
    btnNegativeOffset->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            border-radius: 12px;
        }
    )");

    btnPositiveOffset->setIconSize({32, 32});
    btnPositiveOffset->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            border-radius: 12px;
        }
    )");

    // 偏移 -0.5s 槽函数
    connect(btnNegativeOffset, &QPushButton::clicked, this,
        [this, lyricView](){
        lyricView->addOffset(-500);
    });

    // 偏移 +0.5s 槽函数
    connect(btnPositiveOffset, &QPushButton::clicked, this,
        [this, lyricView](){
        lyricView->addOffset(500);
    });
}