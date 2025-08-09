#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/ImagePool.h>
#include <controller/LyricController.h>
#include <controller/MusicController.h>
#include <cmd/MusicCommand.hpp>
#include <config/MusicConfig.hpp>
#include <utils/SvgPars.hpp>
#include <model/MusicListModel.hpp>

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

    // 注册枚举
    qmlRegisterUncreatableMetaObject(
        HX::PlayModeWrapper::staticMetaObject,
        "HX.Music", // QML import 名称
        1, 0,
        "PlayMode", // QML 中使用名
        "PlayMode 是一个枚举类型, 不可实例化"
    );

    // 注册 音频信息
    qmlRegisterType<HX::MusicInfo>(
        "HX.Music",
        1, 0,
        "MusicInfo"
    );

    QQmlContext* cp = engine.rootContext();
    // 信号总线
    cp->setContextProperty("SignalBusSingleton", &HX::SignalBusSingleton::get());

    // 歌词渲染与管理
    qmlRegisterType<HX::LyricController>(
        "HX.Music", // 导入时候的名称 (import Xxx) (注意得是大写开头)
        1, 0,                       // 主版本号 与 次版本号
        "LyricController"           // qml中使用的组件名称 (注意得是大写开头)
    );

    // 音乐控制
    qmlRegisterType<HX::MusicController>(
        "HX.Music", // 导入时候的名称 (import Xxx) (注意得是大写开头)
        1, 0,                       // 主版本号 与 次版本号
        "MusicController"           // qml中使用的组件名称 (注意得是大写开头)
    );

    // 注册 歌曲列表视图 到 qml
    qmlRegisterType<HX::MusicListModel>(
        "HX.Music",
        1, 0,
        "MusicListModel"
    );

    // 注册img链接: image://musicLyric
    engine.addImageProvider("musicLyric", new HX::LyricController); // 内部会释放!
    
    // 注册img链接: image://svgColored
    engine.addImageProvider("svgColored", new HX::QmlSvgPars); // 内部会释放!

    // 注册img链接: image://imgPool
    engine.addImageProvider("imgPool", HX::ImagePoll::get()); // 内部会释放!

    // 应该使用 _ 和 [0-9a-Z], 不能使用`-`
    engine.loadFromModule("HX.Music", "Main");

    return app.exec();
}
