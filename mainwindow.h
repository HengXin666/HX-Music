#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            isDragging = true;
            dragStartPos = event->globalPosition().toPoint() - this->frameGeometry().topLeft();
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (isDragging && (event->buttons() & Qt::LeftButton)) {
            this->move(event->globalPosition().toPoint() - dragStartPos);
            event->accept();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            isDragging = false;
            event->accept();
        }
    }

private slots:
    void on_exitWindowBtn_clicked();

    void on_toggleMaximizedBtn_clicked();

    void on_showMinimizBtn_clicked();

private:
    Ui::MainWindow *ui;

    bool isDragging = false;
    QPoint dragStartPos; // 记录鼠标拖动起始位置
};
#endif // MAINWINDOW_H
