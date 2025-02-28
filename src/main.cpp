#include <QApplication>
#include <QTextCodec>
#include <window/MainWindow.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    // 强制 Qt 使用 UTF-8 编码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    MainWindow w;
    w.show();
    return a.exec();
}
