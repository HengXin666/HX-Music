#include <widget/VolumeBar.h>

#include <QVBoxLayout>

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>
#include <cmd/MusicCommand.hpp>

VolumeSlider::VolumeSlider(QWidget* parent, QPushButton* btn)
    : QWidget(parent) 
{
    setAttribute(Qt::WA_TranslucentBackground); // 设置没有窗体
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup); // 无边框 + 悬浮窗

    btn->setIcon(QIcon(":/icons/volume_up.svg"));

    _slider->setRange(0, 100);
    _slider->setValue(100 * GlobalSingleton::get().musicConfig.volume);

    _textPercentage->setText(QString("%1%").arg(_slider->value()));

    QVBoxLayout* vBL = new QVBoxLayout(this);
    vBL->addWidget(_textPercentage);
    vBL->addWidget(_slider);
    setLayout(vBL);

    // 预先加载图标, 避免每次都加载文件
    QIcon vLowIcon(":/icons/volume_low.svg");
    QIcon vUpIcon(":/icons/volume_up.svg");
    QIcon vOffIcon(":/icons/volume_off.svg");

    // 拖动条值改变槽
    connect(_slider, &QSlider::valueChanged, this,
        [](int val) {
        MusicCommand::setVolume(val * 0.01f);
    });

    connect(&SignalBusSingleton::get(), &SignalBusSingleton::volumeChanged, this,
    [this, btn,
        vLowIcon = std::move(vLowIcon),
        vUpIcon = std::move(vUpIcon),
        vOffIcon = std::move(vOffIcon)
    ](float volume) {
        int val = volume * 100;
        _textPercentage->setText(QString("%1%").arg(val));
        // 触发修改图标
        // 如果音量在之前的区间则不修改
        if (val >= 50) {
            btn->setIcon(vUpIcon); // 使用缓存的图标
        } else if (val > 0) {
            btn->setIcon(vLowIcon);
        } else {
            btn->setIcon(vOffIcon);
        }
    });
}

VolumeBar::VolumeBar(QWidget* parent)
    : QWidget(parent)
{
    _slider->hide();        // 初始隐藏

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_btn);
    setLayout(layout);

    // 设置 _slider 为相对布局: 相对于 _btn 的上方, 并且不参与碰撞(类似于在窗口顶层)
    connect(_btn, &QPushButton::clicked, this, [this]() {
        if (_slider->isVisible()) {
            _slider->hide();
        } else {
            _slider->show();
            QPoint pos = _btn->mapToGlobal(
                QPoint(
                    (_btn->width() - _slider->width()) / 2,
                    -_slider->height()
                )
            );
            _slider->move(pos);
        }
    });
}
