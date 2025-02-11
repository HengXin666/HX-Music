#include <QApplication>
#include <window/MainWindow.h>

#include <utils/PlayQueue.hpp>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int typeId = qRegisterMetaType<HX::PlayQueue::iterator>("HX::PlayQueue::iterator");
    qDebug() << "Type ID:" << typeId;
    return a.exec();
}