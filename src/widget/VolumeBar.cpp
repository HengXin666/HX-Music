#include <widget/VolumeBar.h>

#include <QVBoxLayout>


VolumeBar::VolumeBar(QWidget* parent)
    : QWidget(parent)
{
    _slider->hide();        // 初始隐藏

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_btn);
    setLayout(layout);

    // 设置 _slider 为相对布局： 相对于 _btn 的上方, 并且不参与碰撞(类似于在窗口顶层)
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
