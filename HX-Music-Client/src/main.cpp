#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/ImagePool.h>
#include <controller/LyricController.h>
#include <controller/MusicController.h>
#include <controller/PlaylistController.h>
#include <cmd/MusicCommand.hpp>
#include <config/MusicConfig.hpp>
#include <config/Theme.hpp>
#include <utils/SvgPars.hpp>
#include <utils/WindowMaskUtil.h>
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

    QQmlContext* cp = engine.rootContext();
    // 信号总线
    cp->setContextProperty("SignalBusSingleton", &HX::SignalBusSingleton::get());

    // 窗口掩码工具类
    HX::WindowMaskUtil windowMaskUtil;
    cp->setContextProperty("WindowMaskUtil", &windowMaskUtil);

    // 主题数据类
    static HX::Theme theme;
    cp->setContextProperty("Theme", &theme);

    // 音乐控制类
    static HX::MusicController musicController;
    cp->setContextProperty("MusicController", &musicController);

    // 注册 歌曲列表视图 到 qml
    qmlRegisterType<HX::MusicListModel>(
        "HX.Music",
        1, 0,
        "MusicListModel"
    );

    // 注册 音频信息
    qmlRegisterType<HX::MusicInfo>(
        "HX.Music",
        1, 0,
        "MusicInfo"
    );

    // 歌词渲染与管理
    auto* lyricController = new HX::LyricController;
    cp->setContextProperty("LyricController", lyricController);
    // 注册img链接: image://musicLyric
    engine.addImageProvider("musicLyric", lyricController); // 内部会释放!
    
    // 注册img链接: image://svgColored
    engine.addImageProvider("svgColored", new HX::QmlSvgPars); // 内部会释放!

    // 注册img链接: image://imgPool
    engine.addImageProvider("imgPool", HX::ImagePoll::get()); // 内部会释放!

    // 应该使用 _ 和 [0-9a-Z], 不能使用`-`
    engine.loadFromModule("HX.Music", "Main");

    HX::PlaylistController playlistController{}; // @todo 加载一下歌单

    // 集中保存
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&] {
        HX::GlobalSingleton::get().musicConfig.position = musicController.getNowPos();
        HX::GlobalSingleton::get().saveConfig();
        lyricController->saveConfig();
    });
    return app.exec();
}
