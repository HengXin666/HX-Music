#include <widget/JumpSlider.h>

#include <QMouseEvent>

void JumpSlider::mousePressEvent(QMouseEvent* ev) {
    switch (orientation()) {
        case Qt::Horizontal: {  // 水平
            int x = ev->pos().x();
            double per = (double)x / width();
            int val = per * (maximum() - minimum()) + minimum();
            setValue(val);
            break;
        }
        case Qt::Vertical: {    // 竖直
            int y = ev->pos().y();
            double per = (double)y / height();
            int val = per * (maximum() - minimum()) + minimum();
            setValue(val);
            break;
        }
    }
    QSlider::mousePressEvent(ev);
}