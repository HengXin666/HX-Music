#include <widget/VolumeBar.h>

VolumeBar::VolumeBar(QWidget* parent)
    : QPushButton(parent)
{
    _slider->setVisible(false);
}