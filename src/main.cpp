#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <singleton/SignalBusSingleton.h>
#include <controller/LyricController.h>
#include <cmd/MusicCommand.hpp>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );

    auto* lyricController = new HX::LyricController;
    QQmlContext* cp = engine.rootContext();
    // 信号总线
    cp->setContextProperty("SignalBusSingleton", &HX::SignalBusSingleton::get());
    qmlRegisterType<HX::LyricController>(
        "HX.Music",
        1, 0,
        "SignalBusSingleton"
    );

    cp->setContextProperty("LyricController", lyricController);
    qmlRegisterType<HX::LyricController>(
        "HX.Music", // 导入时候的名称 (import Xxx) (注意得是大写开头)
        1, 0,                       // 主版本号 与 次版本号
        "LyricController"           // qml中使用的组件名称 (注意得是大写开头)
    );

    // 注册img链接: image://musicLyric
    engine.addImageProvider("musicLyric", lyricController);

    // 应该使用 _ 和 [0-9a-Z], 不能使用`-`
    engine.loadFromModule("HX.Music", "Main");

    // test
    HX::MusicCommand::switchMusic("/run/media/loli/アニメ専門/音乐/いとうかなこ - ファティマ .mp3");
    HX::MusicCommand::resume();
    return app.exec();
}
