#include <widget/VolumeBar.h>

#include <QVBoxLayout>



VolumeBar::VolumeBar(QWidget* parent)
    : QWidget(parent)
{
    _btn->setIcon(QIcon(":/icons/volume_up.svg"));
    _slider->hide();        // 初始隐藏

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_btn);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    {_btn->mapToGlobal(QPoint{0, 0});}

    // 设置 _slider 为相对布局： 相对于 _btn 的上方, 并且不参与碰撞(类似于在窗口顶层)
    connect(_btn, &QPushButton::clicked, this, [this]() {
        if (_slider->isVisible()) {
            _slider->hide();
        } else {
            QPoint pos = _btn->mapToGlobal(QPoint((_btn->width() - _slider->width()) / 2, -_slider->height()));
            _slider->move(pos);
            _slider->show();
            _slider->raise(); // 提升层级，防止被遮挡
            _slider->setFocus(); // 让 slider 获得焦点，这样点击别处时会自动触发 focusOutEvent
        }
    });
}
